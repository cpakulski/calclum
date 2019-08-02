/* This file implements main thread. 
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


void processFiles(int threads_num, std::list<std::string> files) {
  // Create a list with accotiated file contexts.
  std::list<std::tuple<cv::String, std::shared_ptr<CalcLumFileCtx> > > filesToProcess;
  //std::list<std::tuple<int, double> > filesToProcess;
  for (auto file : files) {
    std::tuple<cv::String, std::shared_ptr<CalcLumFileCtx> > t = 
        std::make_tuple(file, std::make_shared<CalcLumFileCtx>());
    filesToProcess.push_back(t);
  }

  // Now create scheduler and variables to feed frames to the scheduler
  CalcLumScheduler s(threads_num);
  s.start();

  // create condition variable to provide feedback from working threads that
  // all frames from a particular file has been processed 
  std::shared_ptr<std::condition_variable> cv = std::make_shared<std::condition_variable>();
  // Mutex is used to:
  // - synchronized access to file context
  // for the coditional variable cv
  std::shared_ptr<std::mutex> cv_m = std::make_shared<std::mutex>();    
  std::shared_ptr<int> files_to_process = std::make_shared<int>(0);

  // Now iterate through all files, read frame by frame and send them to the scheduler for procesing.
  for(auto file : filesToProcess) {
    cv::VideoCapture vc;
    cv::String fileName = std::get<0>(file);
    std::shared_ptr<CalcLumFileCtx> fileCtx = std::get<1>(file);

    if (vc.open(fileName)) {
      printf("Opened %s\n", fileName.c_str());
    } else {
      printf("Failed\n");
    }
 
    fileCtx->setAccessVars(cv, cv_m, files_to_process);
    (*files_to_process)++;

    std::unique_ptr<CalcLumFrameJob> jobToProcess;
    while(true) {
      // create a new frame processing job
      std::unique_ptr<CalcLumFrameJob> newJob = std::make_unique<CalcLumFrameJob>();
      // read new frame to the job class
      if(vc.read(newJob->getFrame())) {
        // we got the next frame. Setup job's fields.
        fileCtx->incFramesRead();
        newJob->setFileCtx(fileCtx);
        //s.addJob(std::move(newJob)); 
        if(nullptr != jobToProcess) {
	  s.addJob(std::move(jobToProcess));
        }
        jobToProcess = std::move(newJob);
      }
      else {
        // mark tht entire file has been read
        fileCtx->setEOF();
        // send the last read frame. EOF is set so, when it is processed we get notified.
        assert(nullptr != jobToProcess);
        s.addJob(std::move(jobToProcess)); 
        printf("Exiting\n");
        break; // exit loop
      }
    }
    vc.release();
  }

  // Now wait on the conditional variable until all files have been processed.
  std::unique_lock<std::mutex> lk(*cv_m);
  printf("Waiting. Files are still %d\n", *files_to_process);
  cv->wait(lk, [files_to_process]{return *files_to_process == 0;});

  printf("I am unlocked\n");
  // now wait until it is finished
  s.stopThreads();

  // Now display all aggregated stats 
  // crete Stats aggregator and add all file contetx
  StatsAggregator aggr;
  for(auto file : filesToProcess) {
    std::shared_ptr<CalcLumFileCtx> file_ctx = std::get<1>(file);
    aggr.addFileCtx(file_ctx); 
  }

  std::cout << "Aggregated statistics across all processed files:" << std::endl;
  std::cout << "  min luminance:    " << aggr.calcMin() << std::endl;
  std::cout << "  max luminance:    " << aggr.calcMax() << std::endl;
  std::cout << "  mean luminance:   " << aggr.calcMean() << std::endl;
  std::cout << "  median luminance: " << aggr.calcMedian() << std::endl;

}

void show_usage(std::string name) {
  std::cout << "Usage: " << name << " -d DIR -t THREADS_NUM" << std::endl;
  std::cout << "       " << "THREADS_NUM is number between 1 and 15" << std::endl;
}

int main(int argc, char* argv[]) {
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
      dir = argv[++i];
    }
  }
  if((0 == threads_num) || dir.empty()) {
    show_usage(argv[0]);
    return 1;
  }
  printf("Running with %d threads\n", threads_num);

  std::list<std::string> files;

  // Open specified directory and find all regular files.
  // do not enter any directories.
  DIR* dirp = opendir(dir.c_str());
  struct dirent * dp;
  while ((dp = readdir(dirp)) != NULL) {
    struct stat file_stat;
    std::string fullPath = dir + std::string("/") + std::string(dp->d_name);
    stat(fullPath.c_str(), &file_stat);
    if(S_ISREG(file_stat.st_mode)) {
      printf("Found file %s\n", dp->d_name);
      files.push_back(fullPath);
    }
  }
  closedir(dirp);

  processFiles(threads_num, files);
  return 1;
  
  // create scheduler with derired number of processing threads
  CalcLumScheduler s(10);
  s.start();

  cv::String filename = "/home/christoph/challenge/video/tears_of_steel_1080p.mov";
  cv::String filename1 = "/home/christoph/challenge/video/ENV087_720p30_LVLH07.ts";
  cv::VideoCapture vc;
 
  // create condition variable to provide feedback from working threads that
  // all frames from a particular file has been processed 
  std::shared_ptr<std::condition_variable> cv = std::make_shared<std::condition_variable>();
  // Mutex is used to:
  // - synchronized access to file context
  // for the coditional variable cv
  std::shared_ptr<std::mutex> cv_m = std::make_shared<std::mutex>();    
  std::shared_ptr<int> files_to_process = std::make_shared<int>(0);


  if (vc.open(filename1)) {
   printf("Opened\n");
   } else {
   printf("Failed\n");
   }
   // create a context representing the file
  std::shared_ptr<CalcLumFileCtx> fileCtx = std::make_shared<CalcLumFileCtx>();
  fileCtx->setAccessVars(cv, cv_m, files_to_process);
  (*files_to_process)++;

  std::unique_ptr<CalcLumFrameJob> jobToProcess;
  while(true) {
    // create a new frame processing job
  std::unique_ptr<CalcLumFrameJob> newJob = std::make_unique<CalcLumFrameJob>();
  // read new frame to the job class
  if(vc.read(newJob->getFrame())) {
    // we got the next frame. Setup job's fields.
    fileCtx->incFramesRead();
    newJob->setFileCtx(fileCtx);
    //s.addJob(std::move(newJob)); 
    if(nullptr != jobToProcess) {
	s.addJob(std::move(jobToProcess));
    }
    jobToProcess = std::move(newJob);
  }
  else {
    // mark tht entire file has been read
    fileCtx->setEOF();
    // send the last read frame. EOF is set so, when it is processed we get notified.
    assert(nullptr != jobToProcess);
    s.addJob(std::move(jobToProcess)); 
    printf("Exiting\n");
    break; // exit loop
  }
  }

  // Now wait on the conditional variable until all files have been processed.
  std::unique_lock<std::mutex> lk(*cv_m);
  printf("Waiting. Files are still %d\n", *files_to_process);
  cv->wait(lk, [files_to_process]{return *files_to_process == 0;});

  printf("I am unlocked\n");
  // now wait until it is finished
  s.stopThreads();
  return 1;
}
