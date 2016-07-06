#include "locationlut.h"
#include <iostream>

constexpr float RANGE = 20.0f;
constexpr float PRECISION = 0.5f;

std::tuple<int, int, int> offsetsForLocation(float x, float y, float z){
  return std::make_tuple((int)x, (int)y, (int)z);
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
  std::tuple<float, float, float, int> entry = lut.at(offsets);
  return std::make_tuple(std::get<0>(entry)/std::get<3>(entry),
			 std::get<1>(entry)/std::get<3>(entry),
			 std::get<2>(entry)/std::get<3>(entry));
}
