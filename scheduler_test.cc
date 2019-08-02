#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "scheduler.h"
#include "job.h"

// Create mock class based on CalcLumJob class
// It is used to make sure that scheduler invokes processJob method
class MockJob : public CalcLumJob {
public:
  MOCK_METHOD0(processJob, void());
};



TEST(Scheduler, CreatingThreads) {
  CalcLumScheduler s(7);

  s.start();
  ASSERT_THAT(s.getThreadsNum(), testing::Eq(7));
  s.stopThreads(); 
}


TEST(Scheduler, AddJob) {
  CalcLumScheduler s(0);
  
  s.addJob(std::make_unique<MockJob>());
  ASSERT_THAT(s.getJobsNum(), testing::Eq(1));
}

TEST(Scheduler, AddMultipleJobs) {
  CalcLumScheduler s(0);

  // do not exeed max outstanding jobs, bacause it will pend.
  int jobs_to_add = s.getMaxOutstandingJobs() - 1;
  for (auto counter = 0; counter < jobs_to_add; counter++) {
    s.addJob(std::make_unique<MockJob>());
  }
  ASSERT_THAT(s.getJobsNum(), testing::Eq(jobs_to_add));
}

TEST(Scheduler, ProcessOneJobOneThread) {
  CalcLumScheduler s(1);

  s.start();
  
  // create a job
  std::unique_ptr<MockJob> job = std::make_unique<MockJob>();
  EXPECT_CALL(*job, processJob());
  
  s.addJob(std::move(job));
  sleep(1); // wait a bit until thread pick the job
  ASSERT_THAT(s.getJobsNum(), testing::Eq(0));
  s.stopThreads();
}

TEST(Scheduler, ProcessManyJobsOneThread) {
  CalcLumScheduler s(1);
  s.start();
  
  for (auto counter = 0; counter < 100; counter++) {
    std::unique_ptr<MockJob> job = std::make_unique<MockJob>();
    EXPECT_CALL(*job, processJob());
  
    s.addJob(std::move(job));
  }
  sleep(1); // wait a bit until thread picks the job
  ASSERT_THAT(s.getJobsNum(), testing::Eq(0));
  s.stopThreads();
}

TEST(Scheduler, ProcessOneJobManyThreads) {
  CalcLumScheduler s(10);
  s.start();
  
  std::unique_ptr<MockJob> job = std::make_unique<MockJob>();
  EXPECT_CALL(*job, processJob());
  
  s.addJob(std::move(job));
  sleep(1); // wait a bit until thread picks the job
  ASSERT_THAT(s.getJobsNum(), testing::Eq(0));
  s.stopThreads();
}

TEST(Scheduler, ProcessManyJobsManyThreads) {
  CalcLumScheduler s(10);
  s.start();
  
  for (auto counter = 0; counter < 100; counter++) {
    std::unique_ptr<MockJob> job = std::make_unique<MockJob>();
    EXPECT_CALL(*job, processJob());
  
    s.addJob(std::move(job));
  }
  sleep(2); // wait a bit until thread picks the job
  ASSERT_THAT(s.getJobsNum(), testing::Eq(0));
  s.stopThreads();
}

int main(int argc, char **argv) {
 ::testing::InitGoogleTest(&argc, argv);
 return RUN_ALL_TESTS();
}

