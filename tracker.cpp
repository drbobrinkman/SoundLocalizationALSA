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

constexpr float CLUSTER_DISTANCE = 0.5f;
constexpr float SMOOTHING_FACTOR = 1.0f/6.0f;
constexpr float TIMEOUT_SECONDS = 1.0f;
constexpr float FPS = 60.0f;
constexpr float TIMEOUT_FRAMES = TIMEOUT_SECONDS * FPS;

Tracker::Tracker(){
}

Tracker::~Tracker(){
}

void Tracker::addPoint(std::vector<float> pt, float loudness, int frameNumber){
  //First, find the closest sound
  float minDist = 100000.0f;
  int minIndex = -1;
  for(int i=0; i<sounds.size(); i++){
    float d = dist(pt, sounds[i].location);
    if(d < minDist){
      minDist = d;
      minIndex = i;
    }
  }

  if(minIndex != -1 && minDist < CLUSTER_DISTANCE){
    //If a good cluster is found, update it

    //Then do a weighted average with the new data. It might be
    // better to weight by loudness than by a constant factor...
    sounds[minIndex].loudness = loudness;
    sounds[minIndex].location = lerp(sounds[minIndex].location, pt,
				  SMOOTHING_FACTOR);
    sounds[minIndex].lastFrame = frameNumber;
  } else {
    //If a matching cluster not found, make a new one
    sounds.push_back(Trackable(pt, frameNumber, frameNumber, loudness));
  }
}

void Tracker::tickUntil(int frameNumber){
  for(int i=sounds.size()-1; i >= 0; i--){
    if(frameNumber - sounds[i].lastFrame >= TIMEOUT_FRAMES){
      sounds.erase(sounds.begin() + i);
    }
  }
}

std::vector<Trackable> Tracker::getSounds(){
  return sounds;
}
  
Trackable::Trackable(std::vector<float> iloc, unsigned long iff,
		     unsigned long ilf, float iloudness) :
  location(iloc), firstFrame(iff), lastFrame(ilf), loudness(iloudness) {
}
