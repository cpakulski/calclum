/*
  Set of unit tests for frame job and statistic calculations.
*/
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "scheduler.h"
#include "frameJob.h"

TEST(frameJob, averageOfOneElement) {
  CalcLumFileCtx file_ctx("test");

  // pretend we finished processing the file
  file_ctx.setEOF();
  file_ctx.incFramesProcessed();
  file_ctx.reportFrameLuminance(21);
  ASSERT_EQ(21, file_ctx.getFileAverageLuminance());
}

TEST(frameJob, averageOf10Elements) {
  CalcLumFileCtx file_ctx("test");

  file_ctx.setEOF();
  for (auto i = 0; i < 10; i++) {
    file_ctx.incFramesProcessed();
    // report every 3 numbers 21, 24, 27, 30, ...
    file_ctx.reportFrameLuminance(21 + i * 3);
  }
  ASSERT_EQ(34, file_ctx.getFileAverageLuminance());
}

TEST(frameJob, minimumLuminanceOneFrame) {
  CalcLumFileCtx file_ctx("test");
  
  file_ctx.setEOF();
  file_ctx.reportFrameLuminance(3);
  ASSERT_EQ(3, file_ctx.getMinLuminance());
}

TEST(frameJob, minimumLuminance50Frames) {
  CalcLumFileCtx file_ctx("test");
  
  file_ctx.setEOF();
  for (auto i = 0; i < 50; i++) {
    file_ctx.reportFrameLuminance(4 + i);
  }
  file_ctx.reportFrameLuminance(2);
  
  ASSERT_EQ(2, file_ctx.getMinLuminance());
}

TEST(frameJob, maximumLuminanceOneFrame) {
  CalcLumFileCtx file_ctx("test");
  
  file_ctx.setEOF();
  file_ctx.reportFrameLuminance(3);
  ASSERT_EQ(3, file_ctx.getMaxLuminance());
}

TEST(frameJob, maximumLuminance50Frames) {
  CalcLumFileCtx file_ctx("test");
  
  file_ctx.setEOF();
  for (auto i = 0; i < 50; i++) {
    file_ctx.reportFrameLuminance(4 + i);
  }
  file_ctx.reportFrameLuminance(101);
  
  ASSERT_EQ(101, file_ctx.getMaxLuminance());
}

TEST(frameJob, medianOneFrame) {
  CalcLumFileCtx file_ctx("test");
  
  file_ctx.setEOF();
  file_ctx.reportFrameLuminance(3);
  ASSERT_EQ(3, file_ctx.getMedianLuminance());
}

// sequnce 3 3 3
TEST(frameJob, medianOf3EqualNums) {
  CalcLumFileCtx file_ctx("test");
  
  file_ctx.setEOF();
  file_ctx.reportFrameLuminance(3);
  file_ctx.reportFrameLuminance(3);
  file_ctx.reportFrameLuminance(3);
  ASSERT_EQ(3, file_ctx.getMedianLuminance());
}

// sequence 3 5 7
TEST(frameJob, medianOf3Nums) {
  CalcLumFileCtx file_ctx("test");
  
  file_ctx.setEOF();
  file_ctx.reportFrameLuminance(3);
  file_ctx.reportFrameLuminance(5);
  file_ctx.reportFrameLuminance(7);
  ASSERT_EQ(5, file_ctx.getMedianLuminance());
}
 // sequence 3 3 3 4 5 5 7 99 101
TEST(frameJob, medianOf9NumsSomeRepeating) {
  CalcLumFileCtx file_ctx("test");
  
  file_ctx.setEOF();
  file_ctx.reportFrameLuminance(3);
  file_ctx.reportFrameLuminance(3);
  file_ctx.reportFrameLuminance(3);
  file_ctx.reportFrameLuminance(4);
  file_ctx.reportFrameLuminance(5);
  file_ctx.reportFrameLuminance(5);
  file_ctx.reportFrameLuminance(7);
  file_ctx.reportFrameLuminance(99);
  file_ctx.reportFrameLuminance(101);
  ASSERT_EQ(5, file_ctx.getMedianLuminance());
}
 
TEST(frameJob, medianOfTwoEqualNumbers) {
  CalcLumFileCtx file_ctx("test");
  
  file_ctx.setEOF();
  file_ctx.reportFrameLuminance(99);
  file_ctx.reportFrameLuminance(99);
  
  ASSERT_EQ(99, file_ctx.getMedianLuminance());
}

// sequence 4 6 should give 5
TEST(frameJob, medianOfTwoDifferentNumbers) {
  CalcLumFileCtx file_ctx("test");
  
  file_ctx.setEOF();
  file_ctx.reportFrameLuminance(4);
  file_ctx.reportFrameLuminance(6);
  
  ASSERT_EQ(5, file_ctx.getMedianLuminance());
}

// sequence  3 4 6 8 should give 5
TEST(frameJob, medianOf4DifferentNumbers) {
  CalcLumFileCtx file_ctx("test");
  
  file_ctx.setEOF();
  file_ctx.reportFrameLuminance(3);
  file_ctx.reportFrameLuminance(4);
  file_ctx.reportFrameLuminance(6);
  file_ctx.reportFrameLuminance(8);
  
  ASSERT_EQ(5, file_ctx.getMedianLuminance());
}

// sequence  4 4 4 4 6 8 should give 4
TEST(frameJob, medianOf6NumbersSomeRepeating) {
  CalcLumFileCtx file_ctx("test");
  
  file_ctx.setEOF();
  file_ctx.reportFrameLuminance(4);
  file_ctx.reportFrameLuminance(4);
  file_ctx.reportFrameLuminance(4);
  file_ctx.reportFrameLuminance(4);
  file_ctx.reportFrameLuminance(6);
  file_ctx.reportFrameLuminance(8);
  
  ASSERT_EQ(4, file_ctx.getMedianLuminance());
}

// sequence  3 4 4 6 8 8 should give 5
TEST(frameJob, medianOf6NumbersSomeRepeating2) {
  CalcLumFileCtx file_ctx("test");
  
  file_ctx.setEOF();
  file_ctx.reportFrameLuminance(3);
  file_ctx.reportFrameLuminance(4);
  file_ctx.reportFrameLuminance(4);
  file_ctx.reportFrameLuminance(6);
  file_ctx.reportFrameLuminance(6);
  file_ctx.reportFrameLuminance(8);
  
  ASSERT_EQ(5, file_ctx.getMedianLuminance());
}

TEST(StatsAggregator, calcMinOneCtx) {
 StatsAggregator aggr;

  // create 1 file context and set minimum value
  std::shared_ptr<CalcLumFileCtx> f = std::make_shared<CalcLumFileCtx>("test");
  f->setEOF();
  f->reportFrameLuminance(1);
  aggr.addFileCtx(f);
  
  ASSERT_EQ(1, aggr.calcMin());
}

TEST(StatsAggregator, calcMinManyCtx) {
 StatsAggregator aggr;

  // create 3 file contexts and set minimum value
  std::shared_ptr<CalcLumFileCtx> f = std::make_shared<CalcLumFileCtx>("test");
  f->setEOF();
  f->reportFrameLuminance(77);
  aggr.addFileCtx(f);
  
  f = std::make_shared<CalcLumFileCtx>("test");
  f->setEOF();
  f->reportFrameLuminance(177);
  aggr.addFileCtx(f);
   
  f = std::make_shared<CalcLumFileCtx>("test");
  f->setEOF();
  f->reportFrameLuminance(33);
  aggr.addFileCtx(f);
   
  f = std::make_shared<CalcLumFileCtx>("test");
  f->setEOF();
  f->reportFrameLuminance(11);
  aggr.addFileCtx(f);
   
  ASSERT_EQ(11, aggr.calcMin());
}

TEST(StatsAggregator, calcMaxOneCtx) {
 StatsAggregator aggr;

  // create 1 file contexts and set maximum value
  std::shared_ptr<CalcLumFileCtx> f = std::make_shared<CalcLumFileCtx>("test");
  f->setEOF();
  f->reportFrameLuminance(17);
  aggr.addFileCtx(f);
  
  ASSERT_EQ(17, aggr.calcMax());
}

TEST(StatsAggregator, calcMaxManyCtx) {
 StatsAggregator aggr;

  // create 3 file contexts and set maximum values
  std::shared_ptr<CalcLumFileCtx> f = std::make_shared<CalcLumFileCtx>("test");
  f->setEOF();
  f->reportFrameLuminance(77);
  aggr.addFileCtx(f);
  
  f = std::make_shared<CalcLumFileCtx>("test");
  f->setEOF();
  f->reportFrameLuminance(177);
  aggr.addFileCtx(f);
   
  f = std::make_shared<CalcLumFileCtx>("test");
  f->setEOF();
  f->reportFrameLuminance(33);
  aggr.addFileCtx(f);
   
  f = std::make_shared<CalcLumFileCtx>("test");
  f->setEOF();
  f->reportFrameLuminance(11);
  aggr.addFileCtx(f);
   
  ASSERT_EQ(177, aggr.calcMax());
}

TEST(StatsAggregator, calcMeanOneCtx) {
 StatsAggregator aggr;

  // create 1 file contexts and 2 values
  std::shared_ptr<CalcLumFileCtx> f = std::make_shared<CalcLumFileCtx>("test");
  f->setEOF();
  f->reportFrameLuminance(180);
  f->incFramesProcessed();
  f->reportFrameLuminance(100);
  f->incFramesProcessed();
  aggr.addFileCtx(f);
  
  ASSERT_EQ(140, aggr.calcMean());
}

TEST(StatsAggregator, calcMeanManyCtx) {
 StatsAggregator aggr;

  // create 3 file contexts and fill with some values
  std::shared_ptr<CalcLumFileCtx> f = std::make_shared<CalcLumFileCtx>("test");
  f->setEOF();
  f->reportFrameLuminance(180);
  f->incFramesProcessed();
  f->reportFrameLuminance(100);
  f->incFramesProcessed();
  aggr.addFileCtx(f);

  f = std::make_shared<CalcLumFileCtx>("test");
  f->setEOF();
  f->reportFrameLuminance(1);
  f->incFramesProcessed();
  f->reportFrameLuminance(17);
  f->incFramesProcessed();
  f->reportFrameLuminance(30);
  f->incFramesProcessed();
  aggr.addFileCtx(f);

  f = std::make_shared<CalcLumFileCtx>("test");
  f->setEOF();
  f->reportFrameLuminance(10);
  f->incFramesProcessed();
  f->reportFrameLuminance(20);
  f->incFramesProcessed();
  f->reportFrameLuminance(80);
  f->incFramesProcessed();
  f->reportFrameLuminance(200);
  f->incFramesProcessed();
  aggr.addFileCtx(f);

  ASSERT_EQ(70, aggr.calcMean());
}

TEST(StatsAggregator, calcMedianOneCtx) {
 StatsAggregator aggr;

  std::shared_ptr<CalcLumFileCtx> f = std::make_shared<CalcLumFileCtx>("test");
  f->reportFrameLuminance(3);
  f->reportFrameLuminance(4);
  f->reportFrameLuminance(6);
  f->reportFrameLuminance(8);
  aggr.addFileCtx(f);
  
  ASSERT_EQ(5, aggr.calcMedian()); 
}

TEST(StatsAggregator, calcMedianManyCtx) {
 StatsAggregator aggr;

  //  create sequence 3 4 6 8
  std::shared_ptr<CalcLumFileCtx> f = std::make_shared<CalcLumFileCtx>("test");
  f->reportFrameLuminance(3);
  f->reportFrameLuminance(4);
  f->reportFrameLuminance(6);
  f->reportFrameLuminance(8);
  aggr.addFileCtx(f);
 
  // Add sequence 4 8 9 1 
  f = std::make_shared<CalcLumFileCtx>("test");
  f->reportFrameLuminance(4);
  f->reportFrameLuminance(8);
  f->reportFrameLuminance(9);
  f->reportFrameLuminance(1);
  aggr.addFileCtx(f);

  // aggreagated sequnce is 1 3 4 4 6 8 8 9. Median is (4+6)/2
  ASSERT_EQ(5, aggr.calcMedian()); 
}

TEST(StatsAggregator, calcMedianManyCtx2) {
 StatsAggregator aggr;

  //  create sequence 3 4 6 8
  std::shared_ptr<CalcLumFileCtx> f = std::make_shared<CalcLumFileCtx>("test");
  f->reportFrameLuminance(3);
  f->reportFrameLuminance(4);
  f->reportFrameLuminance(6);
  f->reportFrameLuminance(8);
  aggr.addFileCtx(f);
 
  // Add sequence 4 9 1 
  f = std::make_shared<CalcLumFileCtx>("test");
  f->reportFrameLuminance(4);
  f->reportFrameLuminance(9);
  f->reportFrameLuminance(1);
  aggr.addFileCtx(f);

  // aggreagated sequnce is 1 3 4 4 6 8 9. Median is 4.
  ASSERT_EQ(4, aggr.calcMedian()); 
}
  
int main(int argc, char **argv) {
 ::testing::InitGoogleTest(&argc, argv);
 return RUN_ALL_TESTS();
}
