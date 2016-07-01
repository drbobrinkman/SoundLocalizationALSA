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

#include <iostream>

#include "microphone.h"
#include "soundProcessing.h"
#include "tracking.h"

int main() {
  Microphone& m = Microphone::getInstance();

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
