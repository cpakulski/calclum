/* 
  This file implements main thread. 
  Steps: 
   - processes command line parameters 
   - creates a list of found files
   - creates scheduler with specified number of threads
   - opens each file and sequentially reads video frame by video frame.
   - frames are packaged into jobs and sent to the scheduler for processing
   - worker threads pick the jobs and process them updating file-specific shared stats
   - when all files has been processed, the main thread waits until worked threads finish
   - aggregated stats for all files are calculated and displayed
*/
#include <unistd.h>
#include "frameJob.h"
#include "scheduler.h"
#include <string>
#include <list>
#include <tuple>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <iostream>

/*
  Function takes list of files to process.
  It opens each file and extracts frame by frame and sends them to the scheduler for procesing. 
*/
int processFiles(int threads_num, std::list<std::string> files) {
  // Create a list with assotiated file contexts.
  std::list<std::tuple<cv::String, std::shared_ptr<CalcLumFileCtx> > > filesToProcess;
  for (auto file : files) {
    std::tuple<cv::String, std::shared_ptr<CalcLumFileCtx> > t = 
        std::make_tuple(file, std::make_shared<CalcLumFileCtx>(file));
    filesToProcess.push_back(t);
  }

  // Now create scheduler
  CalcLumScheduler s(threads_num);
  s.start();

  // create condition variable to provide feedback from working threads that
  // all frames from a particular file has been processed 
  std::shared_ptr<std::condition_variable> cv = std::make_shared<std::condition_variable>();
  // Mutex is used to:
  // - synchronized access to file context
  // - for the condition variable cv
  std::shared_ptr<std::mutex> cv_m = std::make_shared<std::mutex>();    
  std::shared_ptr<int> files_to_process = std::make_shared<int>(0);

  // Now iterate through all files, read frame by frame and send them to the scheduler for processing.
  for(auto file : filesToProcess) {
    cv::VideoCapture vc;
    cv::String fileName = std::get<0>(file);
    std::shared_ptr<CalcLumFileCtx> fileCtx = std::get<1>(file);

    if (!vc.open(fileName)) {
      std::cout << fileName << "->> Invalid file" << std::endl; 
      fileCtx->setError();
      vc.release();
      continue;
    }
 
    fileCtx->setSyncVars(cv, cv_m, files_to_process);
    (*files_to_process)++;

    std::unique_ptr<CalcLumFrameJob> jobToProcess;
    while(true) {
      // create a new frame processing job
      std::unique_ptr<CalcLumFrameJob> newJob = std::make_unique<CalcLumFrameJob>();
      // read new frame to the job class
      if(vc.read(newJob->getFrame())) {
        fileCtx->incFramesRead();
        // we got the next frame. Setup job's fields.
        newJob->setFileCtx(fileCtx);
        if(nullptr != jobToProcess) {
	  s.addJob(std::move(jobToProcess));
        }
        jobToProcess = std::move(newJob);
      }
      else {
        // mark that entire file has been read
        fileCtx->setEOF();
        // send the last read frame. EOF is set now, so we get notified when the last frame has been processed.
        assert(nullptr != jobToProcess);
        s.addJob(std::move(jobToProcess)); 
        break; // exit loop and go to the next file
      }
    }
    vc.release();
  }

  // All frames from all files have been sent to the scheduler.
  // Now wait on the conditional variable until all files have been processed.
  std::unique_lock<std::mutex> lk(*cv_m);
  cv->wait(lk, [files_to_process]{return *files_to_process == 0;});

  s.stopThreads();

  // Now display all aggregated stats 
  // Create stats aggregator and add file contexts for all successfully processed files.
  StatsAggregator aggr;
  for(auto file : filesToProcess) {
    std::shared_ptr<CalcLumFileCtx> file_ctx = std::get<1>(file);
    if(!file_ctx->isError()) {
      aggr.addFileCtx(file_ctx); 
    }
  }

  std::cout << std::endl;
  std::cout << "=================================================" << std::endl;

  if(aggr.empty()) {
    std::cout << "No files were successfully processed" << std::endl;
    return 1;
  }

  std::cout << "Aggregated statistics across all processed files:" << std::endl;
  std::cout << "  min luminance:    " << aggr.calcMin() << std::endl;
  std::cout << "  max luminance:    " << aggr.calcMax() << std::endl;
  std::cout << "  mean luminance:   " << aggr.calcMean() << std::endl;
  std::cout << "  median luminance: " << aggr.calcMedian() << std::endl;
  
  return 0;
}

void show_usage(std::string name) {
  std::cout << "Usage: " << name << " -d DIR -t THREADS_NUM" << std::endl;
  std::cout << "       " << "THREADS_NUM is number between 1 and 15" << std::endl;
}

int main(int argc, char* argv[]) {
  // Command line params processing. In C++ it is always a pain.
  if (argc < 5) {
    show_usage(argv[0]);
    return 1;
  } 

  std::string arg, param;
  auto threads_num = 0;
  std::string dir;
  for(auto i = 0; i < argc; i++) {
    arg = argv[i];
    if(arg == "-t") {
      // next must be number of threads
      param = argv[++i];
      threads_num = std::atoi(param.c_str());
      if ((threads_num < 1) || (threads_num > 15)) {
        show_usage(argv[0]);
        return 1;
      }
    }
    if(arg == "-d") {
      // next must be directory name
      dir = argv[++i];
    }
  }
  if((0 == threads_num) || dir.empty()) {
    show_usage(argv[0]);
    return 1;
  }
  std::cout << "Running with " << threads_num << " threads" << std::endl;

  std::list<std::string> files;

  // Open specified directory and find all regular files.
  // Do not enter any sub-directories.
  DIR* dirp = opendir(dir.c_str());
  if(nullptr == dirp) {
    std::cout << "Cannot access directory " << dir << std::endl;
    return 1; 
  }
  
  struct dirent * dp;
  while ((dp = readdir(dirp)) != NULL) {
    struct stat file_stat;
    std::string fullPath = dir + std::string("/") + std::string(dp->d_name);
    stat(fullPath.c_str(), &file_stat);
    if(S_ISREG(file_stat.st_mode)) {
      std::cout << "Found file " << std::string(dp->d_name) << std::endl;
      files.push_back(fullPath);
    }
  }
  closedir(dirp);
  
  if(files.empty()) {
    std::cout << "No files found ...." << std::endl;
    return 1; 
  }

  std::cout << "Processing files ...." << std::endl;
  return processFiles(threads_num, files);
}
