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
 * Code for this class is based on http://blog.marmakoide.org/?p=1
 **/

#include "spherepoints.h"
#include <cmath>
#include <iostream>

std::vector<std::vector<float> > genPoints(int n){
  std::vector<std::vector<float> > pts;

  float golden_angle = 3.14159265359 * (3 - std::sqrt(5));
  for(int i=0; i < n; i++){
    float theta = i*golden_angle;
    float z = (1 - 1/(float)n)*(1 - (2*i)/(float)(n-1));
    float r = std::sqrt(1 - z*z);
    std::vector<float> pt = {r*std::cos(theta), r*std::sin(theta), z};
    pts.push_back(pt);
  }

  return pts;
}

float maxMinAngle(std::vector<std::vector<float> >& pts){
  float max = 0.0f;
  for(int i=0; i<pts.size(); i++){
    float min = 10000000.0f;
    for(int j=0; j<pts.size(); j++){
      if(j == i) continue;
      //Calculate the angle between the two points, through the origin
      float dotProd = 0;
      for(int k=0; k<3; k++){
	dotProd += pts[i][k]*pts[j][k];
      }
      float len1 = 0;
      float len2 = 0;
      for(int k=0; k<3; k++){
	len1 += pts[i][k]*pts[i][k];
	len2 += pts[j][k]*pts[j][k];
      }
      len1 = std::sqrt(len1);
      len2 = std::sqrt(len2);
      float cosTheta = dotProd/(len1*len2);
      float theta = std::acos(cosTheta);
      if(theta < min){
	min = theta;
      }
    }
    if(min > max){
      max = min;
    }
  }

  return max;
}
/*
int main(){
  std::vector<std::vector<float> > pts = genPoints(256*256);

  std::cout << "spacing: " << maxMinAngle(pts) << std::endl;
  return 0;
  }*/
