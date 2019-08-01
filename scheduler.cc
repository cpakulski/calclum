#include "scheduler.h"

void f(int *p) {
  *p = 99;
}
/*
  Main scheduler. It creates desired number of theads and suumues that each thread
  has the same functionality
*/
int glob;
CalcLumScheduler::CalcLumScheduler(int threads_num) : threads_num_(threads_num) {
  sem_init(&jobs_in_queue_, 0, 0);  

};

CalcLumScheduler::~CalcLumScheduler() {
  for (const std::unique_ptr<std::thread>& it : threads_) {
    // wait until all threads finish
    it->join();
  }
}

void CalcLumScheduler::start() {
  for (auto counter = 0; counter < threads_num_; counter++) {
    //threads_.push_back(std::make_unique<std::thread>(f, &glob));
    threads_.push_back(std::make_unique<std::thread>(processingFunc, this));
  }
}

void CalcLumScheduler::stopThreads() {
  run_.store(false); 
  for (auto counter = 0; counter < threads_num_; counter++) {
    sem_post(&jobs_in_queue_);
  }
}


void CalcLumScheduler::addJob(std::unique_ptr<CalcLumJob> job) {
  std::unique_lock<std::mutex> lck(jobs_list_lock_);
  jobs_list_.push_back(std::move(job));
 
  // signal that there is new job added to the queue
  sem_post(&jobs_in_queue_);
}

int CalcLumScheduler::getJobsNum() {
  std::unique_lock<std::mutex> lck(jobs_list_lock_);
  return jobs_list_.size();
}

void CalcLumScheduler::processingFunc(CalcLumScheduler *s) {
  while(s->run_) {
    // wait for the semaphore to indicate that there is new job in the queue
    sem_wait(&s->jobs_in_queue_);

    std::unique_lock<std::mutex> lck(s->jobs_list_lock_);
    if(0 < s->jobs_list_.size()) {
      std::unique_ptr<CalcLumJob> job = std::move(s->jobs_list_.front());
      s->jobs_list_.pop_front();
    }
    s->jobs_list_lock_.unlock();
  }
}

