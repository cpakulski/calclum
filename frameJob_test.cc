#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "scheduler.h"
#include "frameJob.h"

TEST(frameJob, averageOfOneElement) {
  CalcLumFileCtx file_ctx;

  // pretend we finished processing the file
  file_ctx.setEOF();
  file_ctx.incFramesProcessed();
  file_ctx.reportFrameLuminance(21);
  ASSERT_EQ(21, file_ctx.getFileAverageLuminance());
}

TEST(frameJob, averageOf10Elements) {
  CalcLumFileCtx file_ctx;

  // pretend we finished processing the file
  file_ctx.setEOF();
  for (auto i = 0; i < 10; i++) {
    file_ctx.incFramesProcessed();
    // report every 3 numbers 21, 24, 27, 30, ...
    file_ctx.reportFrameLuminance(21 + i * 3);
  }
  ASSERT_EQ(34, file_ctx.getFileAverageLuminance());
}

TEST(frameJob, minimumLuminanceOneFrame) {
  CalcLumFileCtx file_ctx;
  
  // pretend we finished processing the file
  file_ctx.setEOF();
  file_ctx.reportFrameLuminance(3);
  ASSERT_EQ(3, file_ctx.getMinLuminance());
}

TEST(frameJob, minimumLuminance50Frames) {
  CalcLumFileCtx file_ctx;
  
  // pretend we finished processing the file
  file_ctx.setEOF();
  for (auto i = 0; i < 50; i++) {
    file_ctx.reportFrameLuminance(4 + i);
  }
  file_ctx.reportFrameLuminance(2);
  
  ASSERT_EQ(2, file_ctx.getMinLuminance());
}

TEST(frameJob, maximumLuminanceOneFrame) {
  CalcLumFileCtx file_ctx;
  
  // pretend we finished processing the file
  file_ctx.setEOF();
  file_ctx.reportFrameLuminance(3);
  ASSERT_EQ(3, file_ctx.getMaxLuminance());
}

TEST(frameJob, maximumLuminance50Frames) {
  CalcLumFileCtx file_ctx;
  
  // pretend we finished processing the file
  file_ctx.setEOF();
  for (auto i = 0; i < 50; i++) {
    file_ctx.reportFrameLuminance(4 + i);
  }
  file_ctx.reportFrameLuminance(101);
  
  ASSERT_EQ(101, file_ctx.getMaxLuminance());
}

TEST(frameJob, medianOneFrame) {
  CalcLumFileCtx file_ctx;
  
  // pretend we finished processing the file
  file_ctx.setEOF();
  file_ctx.reportFrameLuminance(3);
  ASSERT_EQ(3, file_ctx.getMedianLuminance());
}

// sequnce 3 3 3
TEST(frameJob, medianOf3EqualNums) {
  CalcLumFileCtx file_ctx;
  
  // pretend we finished processing the file
  file_ctx.setEOF();
  file_ctx.reportFrameLuminance(3);
  file_ctx.reportFrameLuminance(3);
  file_ctx.reportFrameLuminance(3);
  ASSERT_EQ(3, file_ctx.getMedianLuminance());
}

// sequence 3 5 7
TEST(frameJob, medianOf3Nums) {
  CalcLumFileCtx file_ctx;
  
  // pretend we finished processing the file
  file_ctx.setEOF();
  file_ctx.reportFrameLuminance(3);
  file_ctx.reportFrameLuminance(5);
  file_ctx.reportFrameLuminance(7);
  ASSERT_EQ(5, file_ctx.getMedianLuminance());
}
 // sequence 3 3 3 4 5 5 7 99 101
TEST(frameJob, medianOf9NumsSomeRepeating) {
  CalcLumFileCtx file_ctx;
  
  // pretend we finished processing the file
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
  CalcLumFileCtx file_ctx;
  
  // pretend we finished processing the file
  file_ctx.setEOF();
  file_ctx.reportFrameLuminance(99);
  file_ctx.reportFrameLuminance(99);
  
  ASSERT_EQ(99, file_ctx.getMedianLuminance());
}

// sequence 4 6 should give 5
TEST(frameJob, medianOfTwoDifferentNumbers) {
  CalcLumFileCtx file_ctx;
  
  // pretend we finished processing the file
  file_ctx.setEOF();
  file_ctx.reportFrameLuminance(4);
  file_ctx.reportFrameLuminance(6);
  
  ASSERT_EQ(5, file_ctx.getMedianLuminance());
}

// sequence  3 4 6 8 should give 5
TEST(frameJob, medianOf4DifferentNumbers) {
  CalcLumFileCtx file_ctx;
  
  // pretend we finished processing the file
  file_ctx.setEOF();
  file_ctx.reportFrameLuminance(3);
  file_ctx.reportFrameLuminance(4);
  file_ctx.reportFrameLuminance(6);
  file_ctx.reportFrameLuminance(8);
  
  ASSERT_EQ(5, file_ctx.getMedianLuminance());
}

// sequence  4 4 4 4 6 8 should give 4
TEST(frameJob, medianOf6NumbersSomeRepeating) {
  CalcLumFileCtx file_ctx;
  
  // pretend we finished processing the file
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
  CalcLumFileCtx file_ctx;
  
  // pretend we finished processing the file
  file_ctx.setEOF();
  file_ctx.reportFrameLuminance(3);
  file_ctx.reportFrameLuminance(4);
  file_ctx.reportFrameLuminance(4);
  file_ctx.reportFrameLuminance(6);
  file_ctx.reportFrameLuminance(6);
  file_ctx.reportFrameLuminance(8);
  
  ASSERT_EQ(5, file_ctx.getMedianLuminance());
}

int main(int argc, char **argv) {
 ::testing::InitGoogleTest(&argc, argv);
 return RUN_ALL_TESTS();
}
