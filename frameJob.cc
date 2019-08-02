#include "frameJob.h"

/*
  Method precesses a single frame.
  It first computes luinance of the frame
  then it updates the file statistics.
*/
void CalcLumFrameJob::processJob() {
  // frame to be processed is in frame_
  cv::Mat yuv_frame;
  cv::cvtColor(frame_, yuv_frame, CV_BGR2YUV);

  int channels = yuv_frame.channels();
  //printf("Found %d channels\n", channels);
  int rows = yuv_frame.rows;
  int cols = yuv_frame.cols;
  //printf("Found %d cols and %d rows\n", cols, rows);
 
  // point to YUV data
  uint8_t* ptrPixel = yuv_frame.data;
  int pixel_luminance;
  long long frame_luminance = 0;
  cv::Scalar_<uint8_t> yuvPixel;
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      pixel_luminance = ptrPixel[i*cols*channels + j * channels + 0];
      frame_luminance += pixel_luminance;
#if 0
      if(0 != pixel_luminance) {
      printf("Found %d value.\n", pixel_luminance);
       }
#endif
    }
  }
 frame_luminance /= (cols*rows);
 file_ctx_->reportFrameLuminance(frame_luminance);
 //printf("Average frame luminance is %d\n", (int)round(frame_luminance));
 
#if 0
  OK. Here is my attempt to split into color planes and calculate luminance per
  color plane. I must be doing some mistake because it crashes.
  // split it into color planes
  int channels = frame_.channels();
  std::vector<cv::Mat> rgbChannels(channels);
  cv::split(frame_, rgbChannels);

  // Now process each channel
  for (auto color : rgbChannels) {
    // convert the channel to YUV
    cv::Mat yuv_frame;
    cv::cvtColor(frame_, yuv_frame, CV_BGR2YUV);

    int yuv_channels = yuv_frame.channels();
    printf("Found %d channels\n", channels);
    int rows = yuv_frame.rows;
    int cols = yuv_frame.cols;
    printf("Found %d cols and %d rows\n", cols, rows);
 
    // point to YUV data
    uint8_t* ptrPixel = yuv_frame.data;
    int pixel_luminance;
    cv::Scalar_<uint8_t> yuvPixel;
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
        pixel_luminance = ptrPixel[i*cols*channels + j * channels + 0];
        if(0 != pixel_luminance) {
        printf("Found %d value.\n", pixel_luminance);
         }
      }
    }
  }
#endif

  // do nothihng now
  file_ctx_->frames_++;
  //printf("Found %d channels in the frame. Frames: %d\n", channels, file_ctx_->frames_.load());
  file_ctx_->incFramesProcessed();

  file_ctx_->signalEnd();
}


/* This method treis to detect that all frames from a file has been processed.
  It happens only when the reader indicated eof condition and job processor
  has the number of processed frames equal to the number of frames read from the file.
*/
void CalcLumFileCtx::signalEnd() {
  if(!eof_) {
    return;
  }

  if(frames_read_ != frames_processed_) {
    return; 
  }

  // display average file luminance
  std::cout << "Average file luminance: " << getFileAverageLuminance() << std::endl;
  {
    std::lock_guard<std::mutex> lk(*cv_m_);
    (*files_counter_)--;
    printf("Decremented files_counter to %d\n", *files_counter_);
  }
  cv_->notify_all();
}

void CalcLumFileCtx::reportFrameLuminance(int frame_luminance) {
  std::unique_lock<std::mutex> lk(ctx_m_);
  file_luminance_ += frame_luminance;

  // update min luminance
  if(-1 == min_luminance_) {
    min_luminance_ = frame_luminance;
  }
  min_luminance_ = std::min(min_luminance_, frame_luminance);

  // update max luminance
  if (-1 == max_luminance_) {
    max_luminance_ = frame_luminance;
  }
  max_luminance_ = std::max(max_luminance_, frame_luminance);

  // update median_set. Just increase the occurance of the number
  assert(frame_luminance <= 255);
  median_set_[frame_luminance]++;
}

int CalcLumFileCtx::getFileAverageLuminance() {
  // it should never be called before file processing ended.
  assert(eof_);
  return file_luminance_/frames_processed_;
}

int CalcLumFileCtx::getMinLuminance() {
  // it should never be called before file processing ended.
  assert(eof_);
  return min_luminance_;
}

int CalcLumFileCtx::getMaxLuminance() {
  // it should never be called before file processing ended.
  assert(eof_);
  return max_luminance_;
}
 
int CalcLumFileCtx::crunchMedian(std::array<int, 256>& median_set) {
  int total_numbers = 0;
  for (auto it : median_set) {
    total_numbers += it;
  }
  
  int median_loc;
  bool need_two_locs = false;
  if(1 == total_numbers % 2) {
    // this is even number
    median_loc = (total_numbers - 1) /2 + 1;
  } else {
    median_loc = total_numbers / 2;
    need_two_locs = true;
  }

//  printf("Total numbers %d\n", total_numbers);
  int index = 0; 
  while(median_loc > median_set[index]) {
    median_loc -= median_set[index];
  //  printf("Index %d, total_numbers: %d\n", index, median_loc);
    index++;
  }
  if(!need_two_locs) {
    return index;
  }
  // Check if the second index falls into the same bucket as index
  if (median_loc + 1 <= median_set[index]) {
    return index;
  }
  // find the next index
  int second_index = ++index;
  median_loc = 1;
  while(median_loc > median_set[second_index]) {
    median_loc -= median_set[second_index];
    //printf("Index %d, total_numbers: %d\n", index, median_loc);
    second_index++;
  }
  
 return (index + second_index) / 2; 
}

int CalcLumFileCtx::getMedianLuminance() {
  // it should never be called before file processing ended.
  assert(eof_);
  return crunchMedian(median_set_);
}

int StatsAggregator::calcMin() {
  int min = 255;
  // just iterate through all file contexts and find the lowest value
  for (auto file_ctx : files_ctxs_) {
    min = std::min(min, file_ctx->getMinLuminance());
  }
  return min;
}

int StatsAggregator::calcMax() {
  int max = 0;
  // just iterate through all file contexts and find the largest value
  for (auto file_ctx : files_ctxs_) {
    max = std::max(max, file_ctx->getMaxLuminance());
  }
  return max;
}

int StatsAggregator::calcMean() {
  // iterate through all file contexta. From each get number of frames and luminance
  long long total_luminance = 0;
  long long total_frames = 0;
  for (auto file_ctx : files_ctxs_) {
     total_luminance += file_ctx->getFileLuminance();
     total_frames += file_ctx->getFramesProcessed(); 
  }
  return total_luminance/total_frames;
}

int StatsAggregator::calcMedian() {
  std::array<int, 256> total_set;
  total_set.fill(0);

  for (auto file_ctx : files_ctxs_) {
    const std::array<int, 256>& file_set = file_ctx->getMedianSet();
    for (auto index = 0; index < 256; index++) {
      total_set[index] += file_set[index];
    }
  } 
  
  return CalcLumFileCtx::crunchMedian(total_set);
}
