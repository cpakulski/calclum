#include "frameJob.h"

void CalcLumFrameJob::processJob() {
  // do nothihng now
  int channels = frame_.channels();
  file_ctx_->frames_++;
  printf("Found %d channels in the frame. Frames: %d\n", channels, file_ctx_->frames_.load());
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
  {
    std::lock_guard<std::mutex> lk(*cv_m_);
    (*files_counter_)--;
    printf("Decremented files_counter to %d\n", *files_counter_);
  }
  cv_->notify_all();
}
