#include <iostream>

#include "microphone.h"
#include "soundProcessing.h"

int main() {
  Microphone m;

  //Loop forever. Right now, must kill via ctrl-c
  while(true){
    int retVal;

    //Should block if data not yet ready
    retVal = snd_pcm_readi(m.handle, m.buffer, m.frames);
    if(retVal < 0){
      throw std::string("microphone read failed: ") + snd_strerror(retVal);
    }

    float l = estimateLevel(m.buffer, m.frames*m.BYTES_PER_FRAME);
    for(int i=0; i<l; i+=100){
      std::cout << "=";
    }
    std::cout << std::endl;
  }
  
  return 0;
}
