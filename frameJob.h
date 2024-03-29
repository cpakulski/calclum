#pragma once
#include "job.h"
#include <opencv2/opencv.hpp>
#include <atomic>
#include <memory>
#include <condition_variable>
#include <vector>

/*
  CalcLumFileCtx class represents a context releated to a single file.
  There is only one such context per file and it is shared between threads
  processing frames from that file.
*/
class CalcLumFileCtx {
public:
  CalcLumFileCtx() = delete;
  CalcLumFileCtx(const std::string& file_name) : file_name_(file_name) { median_set_.fill(0); }

  void setSyncVars(std::shared_ptr<std::condition_variable> cv,  std::shared_ptr<std::mutex> cv_m,
                   std::shared_ptr<int> files_counter) { cv_ = cv; cv_m_ = cv_m; files_counter_ = files_counter; }
  void incFramesRead() { frames_read_++; }
  void incFramesProcessed() { frames_processed_++; }
  const std::atomic<int>& getFramesRead() const {return frames_read_; }
  int getFramesProcessed() const {return frames_processed_.load(); }
  void signalEnd();
  void setEOF() {eof_ = true;}
  void reportFrameLuminance(int);
  int getFileAverageLuminance();
  int getMinLuminance();
  int getMaxLuminance();
  int getMedianLuminance();
  long long getFileLuminance() const { return file_luminance_; }
  static int crunchMedian(std::array<int, 256>& median_set);
  const std::array<int, 256>& getMedianSet() const {return median_set_; }
  const std::string& getFileName() const {return file_name_; }
  void setError() { error_ = true; }
  bool isError() { return error_; }

private:
  std::string file_name_;
  // The next 3 members are used to indicate that all frames from the file has been processed.
  // They are set by setSyncVars method.
  std::shared_ptr<std::condition_variable> cv_;
  std::shared_ptr<std::mutex> cv_m_;
  std::shared_ptr<int> files_counter_;

  std::atomic<int> frames_read_{0};
  std::atomic<int> frames_processed_{0};
  std::atomic<bool> eof_{false}; // when true it indicates that all frames from file has been read

  // mutex guards access to the per-file statistics
  std::mutex ctx_m_;
  long long file_luminance_{0};
  int min_luminance_{-1};
  int max_luminance_{-1};
  // the following array is used to calculate median value
  // since each luminance is between 0 and 255, the number of occurances of each
  // luminance is stored in array of such size.
  std::array<int, 256> median_set_;

  // set when error happened during processing. It will be omitted
  // when calculating statistics
  bool error_{false};
};

/*
  StatsAggregator class is used to calculate stats across all successfully processed files.
*/
class StatsAggregator {
public:
  void addFileCtx(std::shared_ptr<CalcLumFileCtx> fileCtx) { files_ctxs_.push_back(fileCtx); }
  int calcMin();
  int calcMax();
  int calcMean();
  int calcMedian();
  bool empty() const {return files_ctxs_.empty();}
 
private:
  std::vector<std::shared_ptr<CalcLumFileCtx> > files_ctxs_;
};

/* 
  Job class does specific calculations around the single video frame.
  It is created by main thread, filled with frame data and sent to worker threads
  which execute processJob virtual method. 
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


