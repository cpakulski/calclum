#pragma once
#include "job.h"
#include <opencv2/opencv.hpp>
#include <atomic>
#include <memory>
#include <condition_variable>

/* ths class represents a context releated to a file.
  There is only one such context per file and is shared between threads
  processing frames from theat file.
*/
class CalcLumFileCtx {
public:
  CalcLumFileCtx() {median_set_.fill(0); }
  std::atomic<int> frames_;

  void setAccessVars(std::shared_ptr<std::condition_variable> cv,  std::shared_ptr<std::mutex> cv_m,
                     std::shared_ptr<int> files_counter) { cv_ = cv; cv_m_ = cv_m; files_counter_ = files_counter; }
  void incFramesRead() { frames_read_++; }
  void incFramesProcessed() { frames_processed_++; }
  const std::atomic<int>& getFramesRead() const {return frames_read_; }
  const std::atomic<int>& getFramesProcessed() const {return frames_processed_; }
  void signalEnd();
  void setEOF() {eof_ = true;}
  void reportFrameLuminance(int);
  int getFileAverageLuminance();
  int getMinLuminance();
  int getMaxLuminance();
  int getMedianLuminance();

private:
  // Thge next 3 members are used to indicate that all frames from the file has been processed.
  std::shared_ptr<std::condition_variable> cv_;
  std::shared_ptr<std::mutex> cv_m_;
  std::shared_ptr<int> files_counter_;

  std::atomic<int> frames_read_{0};
  std::atomic<int> frames_processed_{0};
  std::atomic<bool> eof_{false}; // indicates that all frames from file has been read

  // mutex guards access to the per-file statistics
  // info about frames is fetched here
  std::mutex ctx_m_;
  long long file_luminance_{0};
  int min_luminance_{-1};
  int max_luminance_{-1};
  // the following array is used to calculate median value
  // since each luminanace is between 0 and 255, the number of occurances of each
  // luminance is stored in array
  std::array<int, 256> median_set_;
};


/* Job class which does specific calculations around the single video frame. i
*/
class CalcLumFrameJob : public CalcLumJob {
public:

  virtual void processJob() override;
  cv::Mat& getFrame() { return frame_; }
  void setFileCtx(std::shared_ptr<CalcLumFileCtx> file_ctx) { file_ctx_ = file_ctx; }
  virtual ~CalcLumFrameJob() override {}

private:
  cv::Mat frame_;
  std::shared_ptr<CalcLumFileCtx> file_ctx_;
};


