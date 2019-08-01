/* This file implements main thread. 
*/
#include <unistd.h>
#include "frameJob.h"
#include "scheduler.h"


int main() {
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
