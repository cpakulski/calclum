#pragma once

/* 
  Basic abstract class representing a job handled and processed by scheduler.
  It is abstract in order to derive mock class to run unit tests in google framework.
*/
class CalcLumJob {
public:
  virtual void processJob() = 0;
  virtual ~CalcLumJob() {}
};

