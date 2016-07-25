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

#include "soundProcessing.h"
#include "constants.h"
#include "microphone.h"
#include <cmath>
#include <cstdint>
#include <algorithm>

#include <iostream>

float dist(std::vector<float> a, std::vector<float> b){
  float total = 0.0f;
  for(int i=0; i < 3; i++){
    total += (a[i] - b[i])*(a[i] - b[i]);
  }

  return std::sqrt(total);
}

std::vector<float> lerp(std::vector<float> a, std::vector<float> b, float amt){
  //Keep 1-amt of a, add in amt of b
  std::vector<float> ret;
  for(int i=0; i < a.size() && i < b.size(); i++){
    ret.push_back((1.0f-amt)*a[i] + amt*b[i]);
  }

  return ret;
}

constexpr int SIZE = MAX_OFFSET - MIN_OFFSET;

std::vector<std::pair<float, float> > meansAndStdDevs(char* buffer,
						      unsigned int frames){
  float total[4] = {0.0f, 0.0f, 0.0f, 0.0f};
  float totalSq[4] = {0.0f, 0.0f, 0.0f, 0.0f};

  unsigned int count[4] = {0, 0, 0, 0};
  //This assumes that the input format is S16_LE

  int channel = 0;
  for(int i = 0; i < frames*NUM_CHANNELS*BYTES_PER_CHANNEL; i += BYTES_PER_CHANNEL){
    count[channel]++;
    float val = (float)*(int16_t*)(buffer + i);
    total[channel] += val;
    totalSq[channel] += val*val;
    channel = (channel+1)%4;
  }

  std::vector<std::pair<float, float> > ret;
  ret.resize(NUM_CHANNELS);
  for(int i=0; i < NUM_CHANNELS; i++){
    float avg = total[i]/count[i];
    ret[i] = std::make_pair(avg, (float)sqrt(totalSq[i]/count[i] - avg*avg));
  }
  return ret;
}

float diffWithOffset(char* buffer, unsigned int frames,
		     unsigned int ch1, unsigned int ch2,
		     int offset){
  unsigned int count = 0;
  float total = 0.0f;

  for(int i=MAX_OFFSET; i < frames-MAX_OFFSET; i++){
    count++;
    
    float val1 = (float)*(((int16_t*)buffer)+(NUM_CHANNELS*i + ch1));
    float val2 = (float)*(((int16_t*)buffer)+(NUM_CHANNELS*(i+offset) + ch2));

    total += (val1-val2)*(val1-val2);
  }

  float avg = total/count;
  return sqrt(avg);
}

float dotWithOffset(char* buffer, unsigned int frames,
		     unsigned int ch1, unsigned int ch2,
		     int offset){
  unsigned int count = 0;
  float total = 0.0f;

  int ch1offset = 0;
  int ch2offset = 0;
  if(offset >= 0){
    ch2offset = offset;
  } else {
    ch1offset = -offset;
  }
  
  for(int i=0; i < frames-std::abs(offset); i++){
    count++;
    
    float val1 = (float)*(((int16_t*)buffer)+(NUM_CHANNELS*(i+ch1offset)+ch1));
    float val2 = (float)*(((int16_t*)buffer)+(NUM_CHANNELS*(i+ch2offset)+ch2));

    total += val1*val2;
  }

  return total/count;
}

std::vector<std::pair<float, float> > xcorr(char* buffer, unsigned int frames,
					    unsigned int ch1, unsigned int ch2,
					    int range){
  std::vector<std::pair<float, float> > ret;
  
  for(int offset=-range; offset <= range; offset++){
    ret.push_back(std::make_pair(offset,
				 dotWithOffset(buffer, frames, ch1, ch2,
						offset)));
  }

  return ret;
}

int findBestOffset(char* buffer, unsigned int frames,
		   unsigned int ch1, unsigned int ch2){
  static float vals[SIZE];
  
  int bestOffset = MIN_OFFSET;
  float bestVal = diffWithOffset(buffer, frames, ch1, ch2, bestOffset);
  for(int offset=MIN_OFFSET+1; offset < MAX_OFFSET; offset++){
    vals[offset - MIN_OFFSET]= diffWithOffset(buffer, frames, ch1, ch2,
					      offset);
    if(vals[offset - MIN_OFFSET] < bestVal){
      bestVal = vals[offset - MIN_OFFSET];
      bestOffset = offset;
    }
  }

  return bestOffset;
}

void findTopNOffsets(char* buffer, unsigned int frames,
	      unsigned int ch1, unsigned int ch2, int n,
	      std::priority_queue<std::pair<float, int> >& outList){
  while(!outList.empty()){
    outList.pop();
  }

  //Compute the first three, and put them in the heap
  int i = 0;
  for(; i < n && i < frames; i++){
    int offset = MIN_OFFSET + i;
    float val = diffWithOffset(buffer, frames, ch1, ch2, offset);
    outList.push(std::make_pair(val, offset));
  }

  //Now make a new one, then pop the worse one, until done
  for(; i < MAX_OFFSET - MIN_OFFSET; i++){
    int offset = MIN_OFFSET + i;
    float val = diffWithOffset(buffer, frames, ch1, ch2, offset);
    outList.push(std::make_pair(val, offset));
    outList.pop();
  }
}

float diffFourway(char* buffer, unsigned int frames,
		  int offset[4]){
  unsigned int count = 0;
  float total = 0.0f;
  float val[4];
  
  for(int i=MAX_OFFSET; i < frames-MAX_OFFSET; i++){
    count++;
    
    for(int j=0; j<4; j++){
      val[j] = (float)*(((int16_t*)buffer)
			+(NUM_CHANNELS*(i+offset[j]) + j));
    }

    for(int ch1=0; ch1 < 3; ch1++){
      for(int ch2=ch1+1; ch2 < 4; ch2++){
	count++;
	total += (val[ch1]-val[ch2])*(val[ch1]-val[ch2]);
      }
    }
  }

  float avg = total/count;
  return sqrt(avg);
  
}

void recenterAndRescale(char* buffer, unsigned int frames,
			std::vector<std::pair<float, float> > stats){
  float scale[NUM_CHANNELS] = {1.0f, 1.0f, 1.0f, 1.0f};
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
  
  for(int i=0; i<frames; i++){
    for(int j=0; j<NUM_CHANNELS; j++){
      int offset = i*NUM_CHANNELS + j;
      float val = (float)*(((int16_t*)buffer)+offset);
      val = val - stats[j].first;
      val = val*scale[j];
      *(((int16_t*)buffer)+offset) = (int16_t)(val+0.5);
    }
  }
}
