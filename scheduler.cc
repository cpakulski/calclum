#include "scheduler.h"

/*
  Main scheduler class.
*/
CalcLumScheduler::CalcLumScheduler(int threads_num) : threads_num_(threads_num) {
  sem_init(&jobs_in_queue_, 0, 0);  

};

CalcLumScheduler::~CalcLumScheduler() {
  for (const std::unique_ptr<std::thread>& it : threads_) {
    // wait until all threads finish
    it->join();
  }
}

// Start required number of worker threads.
void CalcLumScheduler::start() {
  for (auto counter = 0; counter < threads_num_; counter++) {
    threads_.push_back(std::make_unique<std::thread>(processingFunc, this));
  }
}

void CalcLumScheduler::stopThreads() {
  run_.store(false); 
  for (auto counter = 0; counter < threads_num_; counter++) {
    sem_post(&jobs_in_queue_);
  }
}

/*
  Method is used to add a new job into the scheduler.
  It will pend on condition variable when queue of waiting jobs
  is too large. This is to avoid a situation when worker threads
  cannot keep up with the main thread which creates jobs.
  The queue could grow so large the OOM would kill the process.
  Here we limit the queue to max_outstanding_jobs_ value.
*/
void CalcLumScheduler::addJob(std::unique_ptr<CalcLumJob> job) {
  std::unique_lock<std::mutex> lck(jobs_list_lock_);
  // pend on the condition variable if the queue is too large.
  cv_.wait(lck, [this]{return jobs_list_.size() < max_outstanding_jobs_;});
  jobs_list_.push_back(std::move(job));
 
  // signal that there is new job added to the queue
  sem_post(&jobs_in_queue_);
}

int CalcLumScheduler::getJobsNum() {
  std::unique_lock<std::mutex> lck(jobs_list_lock_);
  return jobs_list_.size();
}

/*
  This is worker thread. It just pends on a semaphore waiting 
  for new job to be added to the queue. Then it takes the job from the 
  queue and processes it.
  If the length os the queue drops below max_outstanding_jobs_ value,
  it will indicate that new jobs can be added to the queue. See addJob method.
*/
void CalcLumScheduler::processingFunc(CalcLumScheduler *s) {
  bool allow_new_jobs ;
  while(s->run_) {
    // wait for the semaphore to indicate that there is new job in the queue
    sem_wait(&s->jobs_in_queue_);

    std::unique_ptr<CalcLumJob> job;
    std::unique_lock<std::mutex> lck(s->jobs_list_lock_);
    allow_new_jobs = (s->jobs_list_.size() < s->max_outstanding_jobs_);
    if(0 < s->jobs_list_.size()) {
      job = std::move(s->jobs_list_.front());
      s->jobs_list_.pop_front();
      // unlock immediately so other threads can continue
      lck.unlock();
      if(allow_new_jobs) {
        s->cv_.notify_all();
      }

      // now just process the job
      job->processJob(); 
    }
    
  }
}

