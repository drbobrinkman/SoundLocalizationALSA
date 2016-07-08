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

/**
 * Portions of this code come from http://www.linuxjournal.com/node/6735/print
 * Tranter, Jeff. "Introduction to Sound Programming with ALSA." Linux Journal,
 *  Sep 30, 2004.
 **/

#pragma once

#include <unordered_map>
#include <tuple>
#include <functional>
#include <vector>

#include "tracking.h"

constexpr float LUT_KEY_PREC = 1.0f;

class LocationLUT {
 public:
  static LocationLUT& getInstance(){
    static LocationLUT instance;
    return instance;
  }
  
 private:
  //This is to make sure we don't try to make LocationLUTs using new and
  // delete
  LocationLUT();

  void buildLUT();
  void loadLUT();
  void saveLUT();

 public:
  //This is to make sure we don't forget to declare our variables
  // as references
  LocationLUT(LocationLUT const&) = delete;
  void operator=(LocationLUT const&) = delete;

  std::tuple<float, float, float, int> get(std::tuple<float, float, float>);
  
 private:
  /**
   * Lookup table. Given three offsets (01, 02, and 03),
   * return the x, y, and z coordinates of the likely source, in meters
   **/
  typedef std::tuple<float, float, float> key_t;

  struct key_hash : public std::unary_function<key_t, std::size_t> {
    std::size_t operator()(const key_t& k) const {
      std::hash<long> long_hash; //TODO: Performance?
  
      long size = LUT_KEY_PREC*(MAX_OFFSET - MIN_OFFSET);
      long val = LUT_KEY_PREC*std::get<0>(k) - LUT_KEY_PREC*MIN_OFFSET;
      val = val*size + (LUT_KEY_PREC*std::get<1>(k) - LUT_KEY_PREC*MIN_OFFSET);
      val = val*size + (LUT_KEY_PREC*std::get<2>(k) - LUT_KEY_PREC*MIN_OFFSET);
      return long_hash(val);
    }
  };
  
  std::unordered_map<std::tuple<float, float, float>,
    std::tuple<float, float, float, int>,
    key_hash> lut;
};
