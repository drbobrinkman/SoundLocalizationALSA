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
#include <iomanip>
#include <fstream>
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

constexpr float RANGE = 10.0f;
constexpr float PRECISION = 0.1f;
constexpr char FNAME[] = "lut.csv";

float dist(std::vector<float> pt1, std::vector<float> pt2){
  float val = 0.0f;
  for(int i=0; i<3; i++){
    val += (pt1[i] - pt2[i])*(pt1[i] - pt2[i]);
  }
  return std::sqrt(val);
}

std::tuple<float, float, float> offsetsForLocation(float x, float y, float z){
  std::vector<float> pt = {x, y, z};

  float micDists[NUM_CHANNELS];
  for(int i=0; i < NUM_CHANNELS; i++){
    micDists[i] = dist(pt, MIC_LOCATIONS[i]);
  }
  
  return std::make_tuple
    (round(LUT_KEY_PREC*((micDists[0] - micDists[1])
			 *SPEED_OF_SOUND_SAMPLES_PER_METER))/LUT_KEY_PREC,
     round(LUT_KEY_PREC*((micDists[0] - micDists[2])
			 *SPEED_OF_SOUND_SAMPLES_PER_METER))/LUT_KEY_PREC,
     round(LUT_KEY_PREC*((micDists[0] - micDists[3])
			 *SPEED_OF_SOUND_SAMPLES_PER_METER))/LUT_KEY_PREC);
}

void LocationLUT::buildLUT(){
  /* First, generate a big list of points */
  std::unordered_map<std::tuple<float, float, float>,
    std::vector<std::tuple<float, float, float>>,
    key_hash> found_points;
  
  static std::vector<float> center = {0.0f, 0.0f, 0.0f};
  
  for(int z = -RANGE/PRECISION; z < RANGE/PRECISION; z++){
    std::cout << "z == " << z << std::endl;
    for(int y = -RANGE/PRECISION; y < RANGE/PRECISION; y++){
      for(int x = -RANGE/PRECISION; x < RANGE/PRECISION; x++){
	std::vector<float> pt = {x*PRECISION, y*PRECISION, z*PRECISION};
	//Don't worry about noises right next to the person
	if(dist(center, pt) < 0.05) continue;
	
	std::tuple<float, float, float> offsets
	  = offsetsForLocation(x*PRECISION, y*PRECISION, z*PRECISION);
	if(found_points.count(offsets) == 0){
	  std::vector<std::tuple<float, float, float>> t;
	  found_points.insert(std::make_pair(offsets, t));
	}

	std::vector<std::tuple<float, float, float>>& entry
	  = found_points.at(offsets);
	/*float mag = std::sqrt(x*x*PRECISION*PRECISION +
			      y*y*PRECISION*PRECISION +
			      z*z*PRECISION*PRECISION);*/
	entry.push_back(std::make_tuple(x*PRECISION, y*PRECISION,
					z*PRECISION));
      }
    }
  }

  /* Now, build the LUT using the found_points */
  for(auto it=found_points.begin(); it!=found_points.end(); ++it){
    std::vector<std::tuple<float, float, float>> &bucket = it->second;

    if(bucket.size() > 0){
      float x=0.0f, y=0.0f, z=0.0f;
      
      for(int i=0; i<bucket.size(); i++){
	std::vector<float> pt = {std::get<0>(bucket[i]),
				 std::get<1>(bucket[i]),
				 std::get<2>(bucket[i])};

	float mag = dist(center, pt);

	x += pt[0]/mag;
	y += pt[1]/mag;
	z += pt[2]/mag;
      }
      
      x /= bucket.size();
      y /= bucket.size();
      z /= bucket.size();

      lut.insert(std::make_pair(it->first,
				std::make_tuple(x, y, z, bucket.size())));
    }
  }
}

void LocationLUT::loadLUT(){
  std::ifstream infile(FNAME);
  if(infile.is_open()){
        std::cout << "loading LUT" << std::endl;
    float floats[6];
    int theint;
    char eatcomma;
    int count;
    std::string trash;
    
    infile >> count;

    //Eat the heading line
    for(int i=0; i<7; i++){
      infile >> trash;
    }
    
    for(int i=0; i<count; i++){
      infile >> floats[0] >> eatcomma
	     >> floats[1] >> eatcomma
	     >> floats[2] >> eatcomma
	     >> floats[3] >> eatcomma
	     >> floats[4] >> eatcomma
	     >> floats[5] >> eatcomma
	     >> theint;
      
      lut.insert(std::make_pair(std::make_tuple(floats[0], floats[1],
						floats[2]),
				std::make_tuple(floats[3], floats[4],
						floats[5], theint)));
    }
  }
  std::cout << "lut size, loaded: " << lut.size() << std::endl;
    
  if(lut.size() == 0){
    buildLUT();
    saveLUT();
  }
  std::cout << "lut size, built: " << lut.size() << std::endl;    
}

void LocationLUT::saveLUT(){
  static std::vector<float> center = {0.0f, 0.0f, 0.0f};

  std::ofstream outfile(FNAME);
  std::cout << "saving LUT" << std::endl;

  outfile << lut.size() << std::endl;
  outfile << "  d01,   d02,   d03,   dir_x,   dir_y,   dir_z, count"
	  << std::endl;
  
  for(auto it=lut.begin(); it!=lut.end(); ++it){
    outfile << std::fixed << std::setprecision(1)
	    << std::setw(5) << std::get<0>(it->first) << ", "
	    << std::setw(5) << std::get<1>(it->first) << ", "
	    << std::setw(5) << std::get<2>(it->first) << ", "
	    << std::setprecision(4)
	    << std::setw(7) << std::get<0>(it->second) << ", "
	    << std::setw(7) << std::get<1>(it->second) << ", "
	    << std::setw(7) << std::get<2>(it->second) << ", "
	    << std::setw(5) << std::get<3>(it->second)
      	    << std::endl;
  }
}

LocationLUT::LocationLUT(){
  loadLUT();
}

std::tuple<float, float, float, int>
LocationLUT::get(std::tuple<float, float, float> offsets){
  std::cout << "lookup: " << "(" << std::get<0>(offsets) << ", "
	    << std::get<1>(offsets) << ", "
	    << std::get<2>(offsets) << ")" << std::endl;
  
  if(lut.count(offsets) > 0){
    std::tuple<float, float, float, int> &entry
      = lut.at(offsets);
    return entry;
  } else {
    //TODO: Spiral out, keep looking
    constexpr float TICK_SIZE = 1/LUT_KEY_PREC;
    int num_ticks = 1;
    while(num_ticks < 10){
      for(int x=-num_ticks; x <= num_ticks; x++){
	for(int y=-num_ticks; y <= num_ticks; y++){
	  for(int z=-num_ticks; z <= num_ticks; z++){
	    std::tuple<float, float, float> trial_offsets
	      = std::make_tuple(std::get<0>(offsets) + x*TICK_SIZE,
				std::get<1>(offsets) + y*TICK_SIZE,
				std::get<2>(offsets) + z*TICK_SIZE);
	    if(lut.count(trial_offsets) > 0){
	      return lut.at(trial_offsets);
	    }
	  }
	}
      }
      num_ticks++;
    }
    return std::make_tuple(10.0f, 10.0f, 10.0f, -1);
  }
}
