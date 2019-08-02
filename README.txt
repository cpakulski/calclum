Installation
------------
The project have been developed on 64-bit Ubuntu 18.04 platform using OpenCV library.

Installation steps for OpenCV library:  
 sudo apt-get update  
 sudo apt-get upgrade  
 sudo apt-get install libopencv-dev  

To install google Test:  
 sudo apt-get install googletest  
  cd /usr/src/googletest/googletest  
  sudo mkdir build  
  cd build  
  sudo cmake ..  
  sudo make           
  sudo cp libgtest* /usr/lib/  
  cd ..  
  sudo rm -rf build  

Compiler:  
 The project have been developed in C++11. I used g++ compiler, but because my system was used for previous projects, I am not sure if the compiler came as default with Ubuntu 18.04 or was installed later on. The version of g++ is:
 g++ --version
   g++ (Ubuntu 7.4.0-1ubuntu1~18.04.1) 7.4.0
   Copyright (C) 2017 Free Software Foundation, Inc.
   This is free software; see the source for copying conditions.  There is NO
   warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

Architecture
------------
The main thread finds all files which need to be processed.
Then it sequentially opens file by file and uses OpenCV methods to read video frame by video frame.
Minimum processing is done on main thread - it only reads frames into memory. Each frame is packaged into a job
and send to scheduler for processing.
The scheduler is created by main thread and consists of the number of worker threads specified in the command line.
The worker threads pick job by job and process each frame in order to calculate luminance value for that frame.
After processing a frame, per-file statistics are updated in the file-specific context. File's context is shared
between worker threads, so updating file's context is controller via mutexes.
Worker threads are agnostic whether jobs belong to a single file or multiple files.

When worker threads finish processing all frames from a file, they signal to main thread that the file processing has been finished.
The main thread waits until all files have been processed and then moves on to calculate aggregate statistics 
across all processed files.

The scheduler contains throttling mechanism to stop adding new jobs into the queue if the queue reaches specified length.
Without that mechanism the queue could grow large if the worked threads cannot keep up with the thread creating new jobs.
This usually happens if the number of worker thread is small (1 or 2) and OOM would kill the process.

Calculating luminance
---------------------
Luminance is calculate by converting a frame to YUV format using OpenCV. 
Then luminance values for each pixel in the frame are added and divided by total number of pixels in the frame.
This gives average luminance value for the frame. Average numbers are rounded to the CLOSEST INTEGER.
I tried to split the frame into color planes, but encountered a problem with OpenCV. That experimental code
is in file frameJob.cc and is commented out.

Building
--------
There is a primitive Makefile included. It contains several targets:
 make clean
  - cleans main executable
 make test
  - compiles and runs all unit tests used in Test Driven Development 
 make calclum
  - builds main executable

Running
-------
Binary needs two parameters:
 - number of threads. This is specified as -t param. 
   Number of threads is capped at 15 and indicate number of worker threads in addition to main thread.
 - directory with files. This is specified as -d param

For example:
  ./calclum -t 7 -d /home/videos

Would launch 7 worker threads and look for files in /home/videos directory.

Known problems
--------------
- OpenCV library displays some warnings when processing .ts files. They can be suppressed by redirecting stderr to /dev/null:
  ./calclum -t 7 -d /home/videos 2>/dev/null

- When invalid file is found, sometimes OpenCV is able to detect it but not always. 
  For example, if you drop calclum binary to the directory with video files, OpenCV will detect that it is invalid file
  and it will be skipped from processing.
  But if you drop an ASCII text file, it was observed that OpenCV reports that it was able to open the file and then it hangs.

