#include <vector>
#include <thread>
#include <semaphore.h>
#include <list>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include "job.h"

/*
  Main scheduler. It creates desired number of theads and suumues that each thread
  has the same functionality
*/
class CalcLumScheduler {
public:
  CalcLumScheduler() = delete;
  CalcLumScheduler(int threads_num);
  ~CalcLumScheduler();

  void start();
  int getThreadsNum() const { return threads_.size(); }

  void addJob(std::unique_ptr<CalcLumJob>);
  int getJobsNum();
  void stopThreads();
 
  int getMaxOutstandingJobs() const { return max_outstanding_jobs_; }

private:
  int threads_num_;
  std::vector<std::unique_ptr<std::thread> > threads_;

  // posix semaphore counting the number of jobs in the queue
  sem_t jobs_in_queue_;

  // list of jobs in the queue
  std::list<std::unique_ptr<CalcLumJob> > jobs_list_;

  // mutex guarding access to the jobs_list_
  std::mutex jobs_list_lock_;
  // condition variable is used as throttle mechanism.
  // rIif the writer is too fast and worker threads are comparatively
  // slow, there would be large number of scheduled jobs waiting in the queue
  // possibly exhausting memory. Therefor only max_outstanding_jobs_ are allowed in
  // the queue.
  std::condition_variable cv_;

  int max_outstanding_jobs_{50};

  // boolean value to indicate that threads should exit
  std::atomic<bool> run_{true};

  static void processingFunc(CalcLumScheduler *);
};
