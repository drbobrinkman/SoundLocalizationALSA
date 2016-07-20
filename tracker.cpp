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

Tracker::Tracker(){
}

Tracker::~Tracker(){
}

void Tracker::addPoint(std::vector<float> pt, float loudness, int frameNumber){
  //First, find the closest sound
  float minDist = 100000.0f;
  int minIndex = -1;
  for(int i=0; i<sounds.size(); i++){
    float d = dist(pt, sounds[i].first);
    if(d < minDist){
      minDist = d;
      minIndex = i;
    }
  }

  if(minIndex != -1 && minDist < CLUSTER_DISTANCE){
    //If a good cluster is found, update it

    //First decay the existing point, if appropriate
    float decay = 1.0f - SMOOTHING_FACTOR;
    while(soundFrameNumbers[minIndex] < frameNumber-1){
      soundFrameNumbers[minIndex]++;
      sounds[minIndex].second *= decay;
    }

    //Then do a weighted average with the new data. It might be
    // better to weight by loudness than by a constant factor...
    sounds[minIndex].second += SMOOTHING_FACTOR*loudness;
    sounds[minIndex].first = lerp(sounds[minIndex].first, pt,
				  SMOOTHING_FACTOR);
    soundFrameNumbers[minIndex] = frameNumber;
  } else {
    //If a matching cluster not found, make a new one
    sounds.push_back(std::make_pair(pt, loudness));
    soundFrameNumbers.push_back(frameNumber);
  }
}

void Tracker::tickUntil(int frameNumber){
  for(int i=sounds.size()-1; i >= 0; i--){
    //For each point in the list, decay it using SMOOTHING_FACTOR for each tick
    float decay = 1.0f - SMOOTHING_FACTOR;
    while(soundFrameNumbers[i] < frameNumber){
      soundFrameNumbers[i]++;
      sounds[i].second *= decay;
    }
    //If gets too quiet, remove
    if(sounds[i].second < SILENCE_LOUDNESS){
      sounds.erase(sounds.begin() + i);
      soundFrameNumbers.erase(soundFrameNumbers.begin() + i);
    }
  }
}

std::vector<std::pair<std::vector<float>, float> > Tracker::getSounds(){
  return sounds;
}
  
