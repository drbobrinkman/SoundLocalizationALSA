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
#include <cmath>

#include "microphone.h"
#include "locationlut.h"
#include "server.h"
#include "soundProcessing.h"
#include "tracking.h"

float dist3(std::vector<float> a, std::vector<float> b){
  float total = 0.0f;
  for(int i=0; i < a.size() && i < b.size(); i++){
    total += (a[i] - b[i])*(a[i] - b[i]);
  }

  return std::sqrt(total);
}

int main() {
  //Build the LUT before opening the mic
  LocationLUT& lut = LocationLUT::getInstance();
  Server& s = Server::getInstance();
  Microphone& m = Microphone::getInstance();

  long frameNumber = 0;
  std::vector<std::vector<float> > recent_pts;
  
  //Loop forever. Right now, must kill via ctrl-c
  while(s.isRunning()){
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
    //Find the loudness of the loudest channel
    float loudness = l[0].second;
    int loudest = 0;
    for(int i=1; i<4; i++){
      if(l[i].second > loudness){
	loudness = l[i].second;
	loudest = i;
      }
    }

    //Then rescale and recenter each signal, so all have
    // mean of 0, and same stdev as loudest signal
    recenterAndRescale(m.buffer.data(), m.frames, l);

    //Find the top 5 possible offsets for each subordinate channel
    // with the lead channel.
    std::priority_queue<std::pair<float, int> > best[4];
    for(int i=0; i<4; i++){
      findTopNOffsets(m.buffer.data(), m.frames, loudest, i, 5, best[i]);
    }

    //Prep the data to be passed to the diffFourWay.
    std::vector<std::vector<int> > offsets;
    for(int i=0; i<4; i++){
      std::vector<int> t;
      offsets.push_back(t);

      while(!best[i].empty()){
	if(i == loudest && offsets[i].size() > 0){
	  //For the loudest signal, we want only the best
	  // offset, which should be 0 ... it will be the
	  // last one out of the priority queue though
	  offsets[i].pop_back();
	}
	offsets[i].push_back((int)best[i].top().second);
	best[i].pop();
      }
    }

    /*for(int i=0; i<offsets.size(); i++){
      for(int j=0; j<offsets[i].size(); j++){
	std::cout << offsets[i][j] << " ";
      }
      std::cout << std::endl;
    }
    std::cout << std::endl;
    */
    //Now do a four-way alignment on the 5^3 candidate offsets
    int besti = -1;
    int bestj = -1;
    int bestk = -1;
    int bestl = -1;
    float bestDiff = 1000000.0f;
    for(int i=0; i<offsets[0].size(); i++){
      for(int j=0; j<offsets[1].size(); j++){
	for(int k=0; k<offsets[2].size(); k++){
	  for(int l=0;l<offsets[3].size(); l++){
	    int tryOffsets[4] = {offsets[0][i],
				 offsets[1][j],
				 offsets[2][k],
				 offsets[3][l]};
	    float diff = diffFourway(m.buffer.data(), m.frames, tryOffsets);
	    if(diff < bestDiff || besti == -1){
	      besti = i;
	      bestj = j;
	      bestk = k;
	      bestl = l;
	      bestDiff = diff;
	    }
	  }
	}
      }
    }

    //The starting value was determined empirically
    static float background_loudness = 100.0f;

    //LUT assumes that stream 0 is the primary stream, so the offets
    // user are 1, 2, and 3 (not 0)
    //Now do a LUT lookup
    std::vector<float> loc = {
      (float)(-offsets[1][bestj] + offsets[0][besti]),
      (float)(-offsets[2][bestk] + offsets[0][besti]),
      (float)(-offsets[3][bestl] + offsets[0][besti])
    };
    std::vector<float> entry = lut.get(loc);

    std::vector<float> cur_pt(entry.begin(), entry.begin()+3);
    static std::vector<float> last_pt = cur_pt;

    //Keep a list of all points found in the last 1/6 of a second
    std::vector<float> new_recent_pt = cur_pt;
    new_recent_pt.push_back(loudness);
    new_recent_pt.push_back(frameNumber);
    recent_pts.push_back(new_recent_pt);
    while(recent_pts.size() > 0 && frameNumber - recent_pts[0][4] > 10){
      recent_pts.erase(recent_pts.begin());
    }
    
    float d = dist3(cur_pt, last_pt);
    
    if(loudness > 300.0f){
      s.putBuffer(m.buffer, loudness, loc, recent_pts);

      std::cout << std::fixed << std::setprecision(2) << std::setw(7)
		<< loudness << " " << bestDiff;
      
      int i=0;
      for(;i<loudness;i+=200){
	std::cout << "=";
      }
      for(;i<3000;i+=200){
	std::cout << " ";
      }
      
      std::cout
	<< "[" << offsets[0][besti] << ", "
	<< offsets[1][bestj] << ", "
	<< offsets[2][bestk] << "] "
	<< "(" << entry[0] << ", "
	<< entry[1] << ", "
	<< entry[2] << ")"
	<< " " << dist3(cur_pt, last_pt)
	<< std::endl;
    }

    frameNumber++;
    last_pt = cur_pt;
  }
  return 0;
}
