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
#include "locationlut.h"
#include "soundProcessing.h"
#include "tracking.h"

int main() {
  Microphone& m = Microphone::getInstance();
  LocationLUT& lut = LocationLUT::getInstance();
  
  //Loop forever. Right now, must kill via ctrl-c
  for(int j=0; j<60*10; j++){
    int retVal;

    //First, read data
    //Should block if data not yet ready
    retVal = snd_pcm_readi(m.handle, m.buffer.data(), m.frames);
    if(retVal < 0){
      throw std::string("microphone read failed: ") + snd_strerror(retVal);
    }

    //Next, calculate mean and stdev for rescaling and centering signals
    std::vector<std::pair<float, float> > l = meansAndStdDevs(m.buffer.data(),
					  m.frames);

    //Then rescale and recenter each signal, so all have
    // mean of 0, and same stdev as loudest signal
    recenterAndRescale(m.buffer.data(), m.frames, l);

    //Find the top 5 possible offsets for each subordinate channel
    // with the lead channel. TODO: Make the loudest channel the lead
    // channel. Right now it is just hard-coded to channel 0.
    std::priority_queue<std::pair<float, int> > best[3];
    for(int i=0; i<3; i++){
      findTopNOffsets(m.buffer.data(), m.frames, 0, i+1, 5, best[i]);
    }

    //Prep the data to be passed to the diffFourWay.
    int offsets[3][5];
    for(int i=0; i<3; i++){
      for(int j=0; j<5; j++){
	offsets[i][j] = best[i].top().second;
	best[i].pop();
      }
    }

    //Now do a four-way alignment on the 5^3 candidate offsets
    int besti = -1;
    int bestj = -1;
    int bestk = -1;
    float bestDiff = 1000000.0f;
    for(int i=0; i<5; i++){
      for(int j=0; j<5; j++){
	for(int k=0; k<5; k++){
	  int tryOffsets[3] = {offsets[0][i], offsets[1][j], offsets[2][k]};
	  float diff = diffFourway(m.buffer.data(), m.frames, tryOffsets);
	  if(diff < bestDiff || besti == -1){
	    besti = i;
	    bestj = j;
	    bestk = k;
	    bestDiff = diff;
	  }
	}
      }
    }

    float loudness = l[0].second;
    for(int i=1; i<4; i++){
      loudness = std::max(loudness, l[i].second);
    }
    int i=0;
    for(;i<loudness;i+=100){
      std::cout << "=";
    }
    for(;i<1500;i+=100){
      std::cout << " ";
    }
    //Now do a LUT lookup
    std::tuple<float, float, float> entry =
      lut.get(std::make_tuple(offsets[0][besti], offsets[1][bestj],
			      offsets[2][bestk]));
    std::cout << "(" << std::get<0>(entry) << ", "
	      << std::get<1>(entry) << ", "
	      << std::get<2>(entry) << ")" << std::endl;
  }
  return 0;
}
