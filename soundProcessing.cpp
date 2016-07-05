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
#include "tracking.h"
#include <cmath>
#include <cstdint>

#include <iostream> //TODO: Remove when done debugging

//TODO: Move these to a shared include
constexpr int BYTES_PER_CHANNEL = 2;
constexpr int NUM_CHANNELS = 4;

//Don't move these, not used outside this file.
constexpr int MAX_OFFSET = (int)(SENSOR_SPACING_SAMPLES*1.25);
constexpr int MIN_OFFSET = -MAX_OFFSET;
constexpr float PRECISION = 0.25;
constexpr int SIZE = MAX_OFFSET - MIN_OFFSET;

float estimateLevel(char* buffer, unsigned int bufferLength){
  float total = 0.0f;
  unsigned int count = 0;
  //This assumes that the input format is S16_LE
  for(int i = 0; i < bufferLength; i += BYTES_PER_CHANNEL){
    count++;
    float val = (float)*(int16_t*)(buffer + i);
    total += val*val;
  }

  float avg = sqrt(total/count);
  return avg;
}

float diffWithOffset(char* buffer, unsigned int frames,
		     unsigned int ch1, unsigned int ch2,
		     int offset){
  unsigned int count = 0;
  float total = 0.0f;

  for(int i=MAX_OFFSET; i < frames-MAX_OFFSET; i++){
    count++;
    
    float val1 = (float)*(((int16_t*)buffer)+NUM_CHANNELS*i + ch1);
    float val2 = (float)*(((int16_t*)buffer)+NUM_CHANNELS*(i+offset) + ch2);

    total += (val1-val2)*(val1-val2);
  }

  float avg = total/count;
  return sqrt(avg);
}

/*
float diffWithOffsetPrecise(char* buffer, unsigned int frames,
		     unsigned int ch1, unsigned int ch2,
		     float foffset){
  unsigned int count = 0;
  float total = 0.0f;
  int offset = (int)foffset;
  float remainder = foffset - offset;
  
  for(int i=MAX_OFFSET; i < frames-MAX_OFFSET-1; i++){
    count++;
    
    float val1 = (float)*(((int16_t*)buffer)+NUM_CHANNELS*i + ch1);
    float val2a = (float)*(((int16_t*)buffer)+NUM_CHANNELS*(i+offset) + ch2);
    float val2b = (float)*(((int16_t*)buffer)+NUM_CHANNELS*(i+offset+1) + ch2);
    float val2 = (1.0-remainder)*val2a + remainder*val2b;
    
    total += (val1-val2)*(val1-val2);
  }

  float avg = total/count;
  return sqrt(avg);
}
*/

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

  //TODO: remove this debug print
  if(ch1 == 0 && ch2 == 1){
    for(int i=0; i<SIZE; i++){
      std::cout << vals[i] << " ";
    }
    std::cout << std::endl << std::endl;
  }
  //END debug print
  
  return bestOffset;
}

void findTopNOffsets(char* buffer, unsigned int frames,
	      unsigned int ch1, unsigned int ch2, int n,
	      std::priority_queue<std::pair<float, int> >& outList){
  while(!outList.empty()){
    outList.pop();
  }

  //Computer the first three, and put them in the heap
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

/*
float findBestOffsetPrecise(char* buffer, unsigned int frames,
		   unsigned int ch1, unsigned int ch2){
  float bestOffset = MIN_OFFSET;
  float bestVal = diffWithOffsetPrecise(buffer, frames, ch1, ch2, bestOffset);
  for(float offset=MIN_OFFSET+PRECISION; offset < MAX_OFFSET;
      offset += PRECISION){
    float val = diffWithOffsetPrecise(buffer, frames, ch1, ch2, offset);
    if(val < bestVal){
      bestVal = val;
      bestOffset = offset;
    }
  }

  return bestOffset;
}
*/

float diffFourway(char* buffer, unsigned int frames,
		  int offset[3]){
  unsigned int count = 0;
  float total = 0.0f;
  float val[4];
  
  for(int i=MAX_OFFSET; i < frames-MAX_OFFSET; i++){
    count++;
    
    val[0] = (float)*(((int16_t*)buffer)+NUM_CHANNELS*i);
    for(int j=1; j<4; j++){
      val[j] = (float)*(((int16_t*)buffer)+NUM_CHANNELS*(i+offset[j-1])
			      + j);
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
