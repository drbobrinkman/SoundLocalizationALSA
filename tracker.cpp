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

#include "tracker.h"
#include "soundProcessing.h"
#include "constants.h"

#include <mutex>

constexpr float CLUSTER_DISTANCE = 0.5f;
constexpr float SMOOTHING_FACTOR = 1.0f/6.0f;
constexpr float TIMEOUT_SECONDS = 1.0f;
constexpr float FPS = TARGET_FRAME_RATE;
constexpr float TIMEOUT_FRAMES = TIMEOUT_SECONDS * FPS;

std::mutex g_sounds_mutex;

Tracker::Tracker(){
}

Tracker::~Tracker(){
}

void Tracker::addPoint(std::vector<float> pt, float loudness,
		       unsigned long frameNumber){
  if(loudness < SILENCE_LOUDNESS) return;
  
  std::lock_guard<std::mutex> guard(g_sounds_mutex);

  //First, find the closest sound that hasn't timed out
  float minDist = 100000.0f;
  int minIndex = -1;
  for(int i=0; i<sounds.size(); i++){
    float d = dist(pt, sounds[i].location);
    if(d < minDist && sounds[i].lastFrame + TIMEOUT_FRAMES >= frameNumber){
      minDist = d;
      minIndex = i;
    }
  }

  if(minIndex != -1 && minDist < CLUSTER_DISTANCE){
    //If a good cluster is found, update it

    //Then do a weighted average with the new data. It might be
    // better to weight by loudness than by a constant factor...
    sounds[minIndex].loudness = std::max(loudness, sounds[minIndex].loudness);
    sounds[minIndex].location = lerp(sounds[minIndex].location, pt,
				  SMOOTHING_FACTOR);
    sounds[minIndex].lastFrame = frameNumber;
  } else {
    //If a matching cluster not found, make a new one
    sounds.push_back(Trackable(pt, frameNumber, frameNumber, loudness));
  }
}

std::vector<Trackable> Tracker::getSoundsSince(unsigned long sFrameNum){
  std::lock_guard<std::mutex> guard(g_sounds_mutex);
  //Remove and sound that hasn't been heard recently
  for(int i=sounds.size()-1; i >= 0; i--){
    if(sounds[i].lastFrame + TIMEOUT_FRAMES < sFrameNum){
      sounds.erase(sounds.begin() + i);
    }
  }
  return sounds;
}
  
Trackable::Trackable(std::vector<float> iloc, unsigned long iff,
		     unsigned long ilf, float iloudness) :
  location(iloc), firstFrame(iff), lastFrame(ilf), loudness(iloudness) {
}
