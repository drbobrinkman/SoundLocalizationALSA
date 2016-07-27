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

#pragma once

#include <vector>

/*! Definition of a trackable object.*/
struct Trackable {
  /*! A 3D unit vector, representing the direction of the sound */
  std::vector<float> location;
  /*! When the sound was first heard at this location */
  unsigned long      firstFrame;
  /*! The last time a sound was heard from this location */
  unsigned long      lastFrame;
  /*! The loudest loudness of this sound over its lifetime */
  float              loudness;

  Trackable(std::vector<float> iloc, unsigned long iff, unsigned long ilf,
	    float iloudness);
};

/*! Manages a collection of Trackable, including clustering nearby sounds
 *  and tracking duration of sounds 
 *
 * \note Singleton, with lazy initialization. (Meyers style singleton)
 *
 * \bug We do not gracefully handle wrap-around or overflow of the frame
 *      numbers. Overflow should take 9 billion years, so we *ought* to be
 *      okay. 
 */
class Tracker {
 public:
  /*! Return the singleton instance. */
  static Tracker& getInstance(){
    static Tracker instance;
    return instance;
  }
  
 private:
  //ctor and dtor are private to encourage correct usage of singleton
  Tracker();
  ~Tracker();

 public:
  /*! Copy ctor deleted so that we don't accidentally make a copy */
  Tracker(Tracker const&) = delete;
  /*! Copy assignment deleted so that we don't accidentally make a copy */
  void operator=(Tracker const&) = delete;

 public:
  /*! Add a newly detected point to the data structure.
   *
   * \param pt A 3D unit vector that indicates the direction of the sound.
   * \param loudness The standard deviation of the signal, which is a rough
   *                 measure of how loud the sound was
   * \param frameNumber Time when this sound was heard, in terms of frames
   *                    of microphone input.
   */
  void addPoint(std::vector<float> pt, float loudness,
		unsigned long frameNumber);

  /*! Get a list of all sounds that have not timed out yet, and
   *  delete those that have timed out.
   *
   *  \param frameNumber Any sound that has been heard recently (within
   * TIMEOUT_FRAMES frames) is returned and kept in the data structure.
   * all others are erased.
   */
  std::vector<Trackable> getSoundsSince(unsigned long frameNumber);
  
 private:
  std::vector<Trackable> sounds;
};
