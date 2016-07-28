/** \file locationlut.h
 * Data structure for mapping sounds delays to directions.
 *
 * We have 4 microphones, and we can calculate the signal delay
 * between microphone 0 and each of the other three. This gives us a 3D
 * vector that tells us something about the location of the sound relative
 * to the four microphones, but not in a form that is usable. What we really
 * want is a unit vector that points in the correct direction from the center
 * of the mic array. This data structure makes it easy to look up the direction
 * given the array of delays.
 *
 * \author Bo Brinkman <dr.bo.brinkman@gmail.com>
 * \date 2016-07-28
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

#include <unordered_map>
#include <functional>
#include <vector>

#include "constants.h"

/*! xyz coordinats used in the keys of the lookup table will be an integer
    multiple of this value */
constexpr float LUT_KEY_PREC = 1.0f;
/*! The maximum delay that might be used in a key in the data structure */
constexpr int MAX_OFFSET = (int)(SENSOR_SPACING_SAMPLES*1.25);
/*! The minimum delay that might be used in a key in the data structure */
constexpr int MIN_OFFSET = -MAX_OFFSET;

/*! A class to build and manage a lookup table to convert an array of
 *  delays into a direction 
 *  \note Singleton, with lazy initialization. (Meyers style singleton) 
 */
class LocationLUT {
 public:
  /*! Return the singleton instance. */
  static LocationLUT& getInstance(){
    static LocationLUT instance;
    return instance;
  }
  
 private:
  //!ctor and dtor are private to encourage correct usage of singleton
  LocationLUT();

  //! Build the lookup table
  void buildLUT();
  //! Load the lookup table from disk, building it if necessary
  void loadLUT();
  //! Save the lookup table to disk
  void saveLUT();

 public:
  /*! Copy ctor deleted so that we don't accidentally make a copy */
  LocationLUT(LocationLUT const&) = delete;
  /*! Copy assignment deleted so that we don't accidentally make a copy */
  void operator=(LocationLUT const&) = delete;

  /*! Given an array of delays, return a unit vector that indicates the
   *  direction
   *
   * \param offsets a length 3 vector, where offest[i] contains the delay
   *        between channel 0 and channel i+1.
   * \return a unit vector that represents the direction of the sound
   */
  std::vector<float> get(std::vector<float> offsets);
  
 private:
  /*! The key type for our lookup table */
  typedef std::vector<float> key_t;

  /*! hash function for use in lookup table */
  struct key_hash : public std::unary_function<key_t, std::size_t> {
    std::size_t operator()(const key_t& k) const {
      std::hash<long> long_hash; //TODO: Performance?
  
      long size = LUT_KEY_PREC*(MAX_OFFSET - MIN_OFFSET);
      long val = LUT_KEY_PREC*k[0] - LUT_KEY_PREC*MIN_OFFSET;
      val = val*size + (LUT_KEY_PREC*k[1] - LUT_KEY_PREC*MIN_OFFSET);
      val = val*size + (LUT_KEY_PREC*k[2] - LUT_KEY_PREC*MIN_OFFSET);
      return long_hash(val);
    }
  };

  /*! The actual lookup table */
  std::unordered_map<std::vector<float>,
    std::vector<float>,
    key_hash> lut;
};
