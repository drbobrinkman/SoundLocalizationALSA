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
#include <iomanip>

#include "microphone.h"
#include "soundProcessing.h"
#include "tracking.h"

int main() {
  Microphone& m = Microphone::getInstance();
  float offsets[4][4];
  float foffsets[4][4];
  
  //Loop forever. Right now, must kill via ctrl-c
  for(int j=0; j<60*5; j++){
    int retVal;

    //Should block if data not yet ready
    retVal = snd_pcm_readi(m.handle, m.buffer.data(), m.frames);
    if(retVal < 0){
      throw std::string("microphone read failed: ") + snd_strerror(retVal);
    }

    std::vector<std::pair<float, float> > l = meansAndStdDevs(m.buffer.data(),
					  m.frames*m.BYTES_PER_FRAME);

    std::cout.write(m.buffer.data(), m.frames*m.BYTES_PER_FRAME);
    /*for(int i=0; i<NUM_CHANNELS; i++){
      std::cout << "(" << l[i].first << ", " << l[i].second << ") ";
    }
    std::cout << std::endl;*/
    /*    int i=0;
    for(; i<l; i+=100){
            std::cout << "=";
    }
    for(; i<1500; i+=100){
        std::cout << " ";
    }
    std::cout << std::fixed << std::setprecision(0)
    << std::setw(4) << l << " ";*/
    /*std::priority_queue<std::pair<float, int> > best[3];
    for(int i=0; i<3; i++){
      findTopNOffsets(m.buffer.data(), m.frames, 0, i+1, 5, best[i]);
    }
    int offsets[3][5];
    for(int i=0; i<3; i++){
      for(int j=0; j<5; j++){
	offsets[i][j] = best[i].top().second;
	best[i].pop();
      }
    }

    for(int i=0; i<5; i++){
      for(int j=0; j<5; j++){
	for(int k=0; k<5; k++){
	  int tryOffsets[3] = {offsets[0][i], offsets[1][j], offsets[2][k]};
	  std::cout << std::setprecision(2)
		    << diffFourway(m.buffer.data(), m.frames, tryOffsets)
		    << ", ";
	}
      }
      std::cout << std::endl;
    }
    std::cout << std::endl; 
    */
    //for(int ch1 = 0; ch1 < 3; ch1++){
      //for(int ch2 = ch1+1; ch2 < 4; ch2++){
	
	//offsets[ch1][ch2] = findBestOffset(m.buffer.data(), m.frames, ch1,
	//				   ch2);

	//std::cout << std::setw(3) << offsets[ch1][ch2] /*<< ", " << std::setw(6) << foffsets[ch1][ch2] */ << " ";
    // }
    //}
    //std::cout << std::endl;
  }

  return 0;
}
