/**
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 **/

#pragma once

#include <vector>

struct Trackable {
  std::vector<float> location;
  unsigned long      firstFrame;
  unsigned long      lastFrame;
  float              loudness;
  Trackable(std::vector<float> iloc, unsigned long iff, unsigned long ilf,
	    float iloudness);
};

class Tracker {
 public:
  static Tracker& getInstance(){
    static Tracker instance;
    return instance;
  }
  
 private:
  //This is to make sure we don't try to make Trackers using new and
  // delete
  Tracker();
  ~Tracker();

 public:
  //This is to make sure we don't forget to declare our variables
  // as references
  Tracker(Tracker const&) = delete;
  void operator=(Tracker const&) = delete;

 public:
  void addPoint(std::vector<float> pt, float loudness, int frameNumber);
  void tickUntil(int frameNumber);
  std::vector<Trackable> getSounds();
  
 private:
  int curFrameNumber = 0;
  std::vector<Trackable> sounds;
};
