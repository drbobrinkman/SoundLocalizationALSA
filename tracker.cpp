/** \file tracker.h
 * Takes in individual detected sounds, and then clusters and tracks them
 * over time.
 * 
 * \author Bo Brinkman <dr.bo.brinkman@gmail.com>
 * \date 2016-07-27
 */

/*
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
#include "constants.h"
#include "utils.h"

#include <mutex>

/*! If a point is within this distance of an existing cluster, it should
 * join that cluster. Note that opposing points have distance 2.0 */
constexpr float CLUSTER_DISTANCE = 0.5f;
/*! When adding new points to a cluster we do a weighted average of
 *  the new location with the old one. This is the amount of weight to
 *  give to the new point */
constexpr float SMOOTHING_FACTOR = 1.0f/6.0f;
/*! If a cluster hasn't been heard in this amount of time, remove from
 *  the list.*/
constexpr float TIMEOUT_SECONDS = 1.0f;
/*! Timeout in frames instead of seconds, for convenience */
constexpr float TIMEOUT_FRAMES = TIMEOUT_SECONDS * TARGET_FRAME_RATE;

/*! addPoint and getSoundsSince are called from different threads, so
 *  we need to guard with a mutex */
std::mutex g_sounds_mutex;

/*! Linear interpolation between two vectors. The length of the resulting
 *  vector is the minimum of the lenghts of the input vectors
 *
 *  \param amt Amount of vector b to include. For example, 
 *  1.0 gives just vector b, 0.0
 *  gives vector a, and 0.5 gives the average of the two vectors.
 *  \param a The first vector, expected to have length 3
 *  \param b The second vector, expected to have length 3
 */
std::vector<float> lerp(std::vector<float> a, std::vector<float> b, float amt){
  //Keep 1-amt of a, add in amt of b
  std::vector<float> ret;
  for(int i=0; i < a.size() && i < b.size(); i++){
    ret.push_back((1.0f-amt)*a[i] + amt*b[i]);
  }

  return ret;
}

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
