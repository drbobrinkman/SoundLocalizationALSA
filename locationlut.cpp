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

#include "locationlut.h"
#include "microphone.h"

#include <iostream>
#include <vector>
#include <cmath>

//Origin is on the triangle, directly below the "up" mic
std::vector<std::vector<float> > MIC_LOCATIONS =
  {
    {0.0f, SENSOR_SPACING_METERS/(2*SIN_60), 0.0f}, //Front
    { SENSOR_SPACING_METERS/2, -SENSOR_SPACING_METERS/(2*TAN_60), 0.0f}, //R
    {-SENSOR_SPACING_METERS/2, -SENSOR_SPACING_METERS/(2*TAN_60), 0.0f}, //L
    {0.0f, 0.0f, SENSOR_SPACING_METERS}  //Up
  };

constexpr float RANGE = 5.0f;
constexpr float PRECISION = 0.01f;

float dist(std::vector<float> pt1, std::vector<float> pt2){
  float val = 0.0f;
  for(int i=0; i<3; i++){
    val += (pt1[i] - pt2[i])*(pt1[i] - pt2[i]);
  }
  return std::sqrt(val);
}

std::tuple<int, int, int> offsetsForLocation(float x, float y, float z){
  std::vector<float> pt = {x, y, z};

  float micDists[NUM_CHANNELS];
  for(int i=0; i < NUM_CHANNELS; i++){
    micDists[i] = dist(pt, MIC_LOCATIONS[i]);
  }
  
  return std::make_tuple
    ((int)((micDists[0] - micDists[1])*SPEED_OF_SOUND_SAMPLES_PER_METER+0.5f),
     (int)((micDists[0] - micDists[2])*SPEED_OF_SOUND_SAMPLES_PER_METER+0.5f),
     (int)((micDists[0] - micDists[3])*SPEED_OF_SOUND_SAMPLES_PER_METER+0.5f));
}

void LocationLUT::buildLUT(){
  for(float z = -RANGE; z < RANGE; z += PRECISION){
    std::cout << "z == " << z << std::endl;
    for(float y = -RANGE; y < RANGE; y += PRECISION){
      for(float x = -RANGE; x < RANGE; x += PRECISION){
	std::tuple<int, int, int> offsets = offsetsForLocation(x, y, z);
	if(lut.count(offsets) == 0){
	  lut.insert(std::make_pair(offsets, std::make_tuple(x, y, z, 1)));
	} else {
	  std::tuple<float, float, float, int>& entry = lut.at(offsets);
	  std::get<0>(entry) += x;
	  std::get<1>(entry) += y;
	  std::get<2>(entry) += z;
	  std::get<3>(entry) += 1;
	}
      }
    }
  }
  std::cout << "lut size: " << lut.size() << std::endl;
}

void LocationLUT::loadLUT(){
  //TODO: Load LUT from disk here
  if(lut.size() == 0){
    buildLUT();
    saveLUT();
  }
}

void LocationLUT::saveLUT(){
  //TODO: Save LUT to disk here
}

LocationLUT::LocationLUT(){
  loadLUT();
}

std::tuple<float, float, float>
LocationLUT::get(std::tuple<int, int, int> offsets){
  //TODO: If space is empty, spiral out to nearby items
  if(lut.count(offsets) > 0){
    std::tuple<float, float, float, int> entry = lut.at(offsets);
    return std::make_tuple(std::get<0>(entry)/std::get<3>(entry),
			   std::get<1>(entry)/std::get<3>(entry),
			   std::get<2>(entry)/std::get<3>(entry));
  } else {
    return std::make_tuple(0.0f, 0.0f, 0.0f);
  }
}
