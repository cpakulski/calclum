#pragma once

/* Basic wrapper around the job.
  contains data and method to call on that data.
*/
class CalcLumJob {
public:
  virtual void processJob() = 0;
  virtual ~CalcLumJob() {}
};

