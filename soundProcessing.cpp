#include "soundProcessing.h"
#include <cmath>
#include <cstdint>

float estimateLevel(char* buffer, int bufferLength){
  float total = 0.0f;
  unsigned int count = 0;
  //This assumes that the input format is S16_LE
  for(int i = 0; i < bufferLength; i += 2){
    count++;
    float val = (float)*(int16_t*)(buffer + i);
    total += val*val;
  }

  float avg = sqrt(total/count);
  return avg;
}
