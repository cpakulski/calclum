#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "scheduler.h"

TEST(Scheduler, CreatingThreads) {
  CalcLumScheduler s(7);

  s.start();
  ASSERT_THAT(s.getThreadsNum(), testing::Eq(7));
  s.stopThreads(); 
}


TEST(Scheduler, AddJob) {
  CalcLumScheduler s(0);
  
  s.addJob(std::make_unique<CalcLumJob>());
  ASSERT_THAT(s.getJobsNum(), testing::Eq(1));
}

TEST(Scheduler, AddMultipleJobs) {
  CalcLumScheduler s(0);
  
  for (auto counter = 0; counter < 100; counter++) {
    s.addJob(std::make_unique<CalcLumJob>());
  }
  ASSERT_THAT(s.getJobsNum(), testing::Eq(100));
}

TEST(Scheduler, ProcessOneJobOneThread) {
  CalcLumScheduler s(1);
  s.start();
  
  s.addJob(std::make_unique<CalcLumJob>());
  sleep(1); // wait a bit until thread pick the job
  ASSERT_THAT(s.getJobsNum(), testing::Eq(0));
  s.stopThreads();
}

TEST(Scheduler, ProcessManyJobsOneThread) {
  CalcLumScheduler s(1);
  s.start();
  
  for (auto counter = 0; counter < 100; counter++) {
    s.addJob(std::make_unique<CalcLumJob>());
  }
  sleep(1); // wait a bit until thread picks the job
  ASSERT_THAT(s.getJobsNum(), testing::Eq(0));
  s.stopThreads();
}

TEST(Scheduler, ProcessOneJobManyThreads) {
  CalcLumScheduler s(10);
  s.start();
  
  s.addJob(std::make_unique<CalcLumJob>());
  sleep(1); // wait a bit until thread picks the job
  ASSERT_THAT(s.getJobsNum(), testing::Eq(0));
  s.stopThreads();
}

TEST(Scheduler, ProcessManyJobsManyThreads) {
  CalcLumScheduler s(10);
  s.start();
  
  for (auto counter = 0; counter < 100; counter++) {
    s.addJob(std::make_unique<CalcLumJob>());
  }
  sleep(2); // wait a bit until thread picks the job
  ASSERT_THAT(s.getJobsNum(), testing::Eq(0));
  s.stopThreads();
}

int main(int argc, char **argv) {
 ::testing::InitGoogleTest(&argc, argv);
 return RUN_ALL_TESTS();
}

