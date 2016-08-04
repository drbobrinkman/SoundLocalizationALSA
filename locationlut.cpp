/** \file locationlut.cpp
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

#include "locationlut.h"
#include "spherepoints.h"
#include "utils.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <cmath>

/*! Sensor spacing converted to meters */
constexpr float SENSOR_SPACING_METERS = SENSOR_SPACING_INCHES*METERS_PER_INCH;

/*! Speed of sound in terms of how many microphone samples elapse as sound
 * travels one meter */
constexpr float SPEED_OF_SOUND_SAMPLES_PER_METER = SAMPLES_PER_SECOND *
  SPEED_OF_SOUND_SECONDS_PER_METER;

/*! sin of 60 degrees */
constexpr float SIN_60 = 0.86602540378f;
/*! tan of 60 degrees */
constexpr float TAN_60 = 1.73205080757f;

/*! Locations of the 4 microphones relative to the origin, in meters.
 *
 * Microphones form a tetrahedron, with three mics in a plane parallel
 * to the ground, and one mic above them. The origin is in the plane parallel
 * to the ground, directly under the "up" mic.
 */
std::vector<std::vector<float> > MIC_LOCATIONS =
  {
    {0.0f, SENSOR_SPACING_METERS/(2*SIN_60), 0.0f}, //Front
    {-SENSOR_SPACING_METERS/2, -SENSOR_SPACING_METERS/(2*TAN_60), 0.0f}, //L
    { SENSOR_SPACING_METERS/2, -SENSOR_SPACING_METERS/(2*TAN_60), 0.0f}, //R
    {0.0f, 0.0f, SENSOR_SPACING_METERS}  //Up
  };

/*! Name of file for caching the lookup table */
constexpr char FNAME[] = "lut.csv";

/*! Given a location in world coordinates, calculate the microphone
 *  delay offsets for mics 1, 2, and 3.
 *
 * \param pt coordinates of the point, in meters
 */
std::vector<float> offsetsForLocation(std::vector<float> pt){
  float micDists[MIC_LOCATIONS.size()];
  for(int i=0; i < MIC_LOCATIONS.size(); i++){
    micDists[i] = dist(pt, MIC_LOCATIONS[i]);
  }
  
  std::vector<float> ret = {
    (float)round(LUT_KEY_PREC*((micDists[0] - micDists[1])
			 *SPEED_OF_SOUND_SAMPLES_PER_METER))/LUT_KEY_PREC,
     (float)round(LUT_KEY_PREC*((micDists[0] - micDists[2])
			 *SPEED_OF_SOUND_SAMPLES_PER_METER))/LUT_KEY_PREC,
     (float)round(LUT_KEY_PREC*((micDists[0] - micDists[3])
			 *SPEED_OF_SOUND_SAMPLES_PER_METER))/LUT_KEY_PREC};

  return ret;
}

void LocationLUT::buildLUT(){
  /* First, generate a big list of points */
  std::unordered_map<std::vector<float>,
    std::vector<std::vector<float>>,
    key_hash> found_points;
  
  static std::vector<float> center = {0.0f, 0.0f, 0.0f};

  //A sphere with 64k points on it, each point should be spaced
  // about one degree apart
  std::vector<std::vector<float> > pts = genPoints(256*256);

  float scale = 1.5;
  
  for(int i=0; i<pts.size(); i++){
    std::vector<float> pt
      = {scale*pts[i][0], scale*pts[i][1], scale*pts[i][2]};
    
    //Don't worry about noises right next to the person
    if(dist(center, pt) < 0.05) continue;
	
    std::vector<float> offsets = offsetsForLocation(pt);
    
    if(found_points.count(offsets) == 0){
      std::vector<std::vector<float>> t;
      //Add empty tuple, to modify in a sec
      found_points.insert(std::make_pair(offsets, t));
    }

    std::vector<std::vector<float>>& entry
      = found_points.at(offsets);
    
    entry.push_back(pt);
  }

  /* Now, build the LUT using the found_points */
  for(auto it=found_points.begin(); it!=found_points.end(); ++it){
    std::vector<std::vector<float>> &bucket = it->second;

    if(bucket.size() > 0){
      float x=0.0f, y=0.0f, z=0.0f;
      
      for(int i=0; i<bucket.size(); i++){
	std::vector<float> pt(bucket[i].begin(), bucket[i].begin()+3);

	float mag = dist(center, pt);

	x += pt[0]/mag;
	y += pt[1]/mag;
	z += pt[2]/mag;
      }
      
      x /= bucket.size();
      y /= bucket.size();
      z /= bucket.size();

      std::vector<float> newent = {
	(float)x, (float)y, (float)z, (float)bucket.size()
      };
      lut.insert(std::make_pair(it->first, newent));
    }
  }
}

void LocationLUT::loadLUT(){
  std::ifstream infile(FNAME);
  if(infile.is_open()){
    //std::cout << "loading LUT" << std::endl;
    float floats[7];
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
	     >> floats[6];

      std::vector<float> key = {
	floats[0], floats[1], floats[2]
      };
      std::vector<float> ent = {
	floats[3], floats[4],floats[5], floats[6]
      };
      lut.insert(std::make_pair(key, ent));
    }
  }
  //std::cout << "lut size, loaded: " << lut.size() << std::endl;
    
  if(lut.size() == 0){
    buildLUT();
    saveLUT();
  }
  //std::cout << "lut size, built: " << lut.size() << std::endl;    
}

void LocationLUT::saveLUT(){
  static std::vector<float> center = {0.0f, 0.0f, 0.0f};

  std::ofstream outfile(FNAME);
  //std::cout << "saving LUT" << std::endl;

  outfile << lut.size() << std::endl;
  outfile << "  d01,   d02,   d03,   dir_x,   dir_y,   dir_z, count"
	  << std::endl;
  
  for(auto it=lut.begin(); it!=lut.end(); ++it){
    outfile << std::fixed << std::setprecision(1)
	    << std::setw(5) << (it->first)[0] << ", "
	    << std::setw(5) << (it->first)[1] << ", "
	    << std::setw(5) << (it->first)[2] << ", "
	    << std::setprecision(4)
	    << std::setw(7) << (it->second)[0] << ", "
	    << std::setw(7) << (it->second)[1] << ", "
	    << std::setw(7) << (it->second)[2] << ", "
	    << std::setw(5) << (it->second)[3]
      	    << std::endl;
  }
}

LocationLUT::LocationLUT(){
  loadLUT();
}

std::vector<float>
LocationLUT::get(std::vector<float> offsets){
  if(lut.count(offsets) > 0){
    std::vector<float> &entry
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
	    std::vector<float> trial_offsets = {
	      offsets[0] + x*TICK_SIZE,
	      offsets[1] + y*TICK_SIZE,
	      offsets[2] + z*TICK_SIZE
	    };
	    if(lut.count(trial_offsets) > 0){
	      return lut.at(trial_offsets);
	    }
	  }
	}
      }
      num_ticks++;
    }
    std::vector<float> ret = {10.0f, 10.0f, 10.0f, -1.0f};
    return ret;
  }
}
