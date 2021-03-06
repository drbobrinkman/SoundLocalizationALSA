/** \file soundProcessing.cpp
 * Collection of functions for various kinds of sound processing.
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

#include "soundProcessing.h"
#include "constants.h"
#include <cmath> //For sqrt, abs, and so on
#include <cstdint> //For int16_t

std::vector<std::pair<float, float> >
meansAndStdDevs(const std::vector<int16_t>& buffer){
  std::vector<float> total(NUM_CHANNELS, 0.0f);
  std::vector<float> totalSq(NUM_CHANNELS, 0.0f);

  std::vector<unsigned int> count(NUM_CHANNELS, 0);
  //This assumes that the input format is S16_LE

  int channel = 0;
  for(int i = 0; i < buffer.size(); i++){
    count[channel]++;
    float val = (float)buffer[i];
    total[channel] += val;
    totalSq[channel] += val*val;
    channel = (channel+1)%NUM_CHANNELS;
  }

  std::vector<std::pair<float, float> > ret;
  ret.resize(NUM_CHANNELS);
  for(int i=0; i < NUM_CHANNELS; i++){
    float avg = total[i]/count[i];
    ret[i] = std::make_pair(avg, (float)sqrt(totalSq[i]/count[i] - avg*avg));
  }
  return ret;
}

float dotWithOffset(const std::vector<int16_t>& buffer,
		     unsigned int ch1, unsigned int ch2,
		     int offset){
  unsigned int count = 0;
  float total = 0.0f;
  unsigned int frames = buffer.size() / NUM_CHANNELS;

  int ch1offset = 0;
  int ch2offset = 0;
  if(offset >= 0){
    ch2offset = offset;
  } else {
    ch1offset = -offset;
  }
  
  for(int i=0; i < frames-std::abs(offset); i++){
    count++;
    
    float val1 = (float)buffer[NUM_CHANNELS*(i+ch1offset)+ch1];
    float val2 = (float)buffer[NUM_CHANNELS*(i+ch2offset)+ch2];

    total += val1*val2;
  }

  return total/count;
}

std::vector<std::pair<float, float> >
xcorr(const std::vector<int16_t>& buffer,
      unsigned int ch1, unsigned int ch2,
      int range){
  std::vector<std::pair<float, float> > ret;
  
  for(int offset=-range; offset <= range; offset++){
    ret.push_back(std::make_pair(offset, dotWithOffset(buffer, ch1, ch2,
						       offset)));
  }

  return ret;
}

std::pair<float, float> delay(const std::vector<int16_t>& buffer,
			      unsigned int ch1, unsigned int ch2,
			      int range){
  std::vector<std::pair<float, float> > corrs
    = xcorr(buffer, ch1, ch2, range+2); //TODO: Should this +2 be gone now?

  float ac1 = dotWithOffset(buffer, ch1, ch1, 0);
  float ac2 = dotWithOffset(buffer, ch2, ch2, 0);
  
  int maxOffset = 0;
  std::pair<float, float> maxVal = corrs[0];
  
  for(int i=1; i<corrs.size(); i++){
    if(corrs[i].second > maxVal.second){
      maxVal = corrs[i];
      maxOffset = i;
    } else if(corrs[i].second == maxVal.second &&
	      std::abs(corrs[i].first) < std::abs(maxVal.first)){
      maxVal = corrs[i];
      maxOffset = i;
    }
  }

  return std::make_pair(maxVal.first, maxVal.second/std::sqrt(ac1*ac2));
}

void recenter(std::vector<int16_t>& buffer, 
	      std::vector<std::pair<float, float> > stats){
  std::vector<float> scale(NUM_CHANNELS, 1.0f);
  float amax = stats[0].second;
  for(int i=1; i<NUM_CHANNELS; i++){
    amax = std::max(amax, stats[i].second); 
  }
  
  for(int i=0; i<NUM_CHANNELS; i++){
    //If the stdev is less than 1.0f, something went really wrong. Mics
    // are noisier than that.
    if(stats[i].second > 0.0f){
      scale[i] = amax/stats[i].second;
    }
  }

  unsigned int frames = buffer.size() / NUM_CHANNELS;

  for(int i=0; i<frames; i++){
    for(int j=0; j<NUM_CHANNELS; j++){
      int offset = i*NUM_CHANNELS + j;
      float val = (float)buffer[offset];
      val = val - stats[j].first;
      //val = val*scale[j]; //Only needed if we also want to rescale
      buffer[offset] = (int16_t)(val+0.5);
    }
  }
}
