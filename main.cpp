/** \file main.cpp
 * Main controller for the whole project.
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

#include <iostream>
#include <iomanip>
#include <cmath>

#include "microphone.h"
#include "locationlut.h"
#include "server.h"
#include "soundProcessing.h"
#include "constants.h"
#include "tracker.h"
#include "updateServer.h"
#include "utils.h"

/*! Main controller method for the whole project */
int main() {
  std::cout << "updating IP Discovery Server" << std::endl;
  updateIPDiscoveryServer();
  
  //Build the LUT before opening the mic
  std::cout << "building LUT" << std::endl;
  LocationLUT& lut = LocationLUT::getInstance();

  std::cout << "creating Tracker" << std::endl;
  Tracker& t = Tracker::getInstance();

  std::cout << "creating Server" << std::endl;
  Server& s = Server::getInstance(t);

  std::cout << "creating Microphone" << std::endl;
  Microphone& m = Microphone::getInstance();
  
  long frameNumber = 0;

  std::cout << "main loop starting" << std::endl;
  //Loop forever. Right now, must kill via ctrl-c
  while(s.isRunning()){
    int retVal;

    //First, read data
    //Should block if data not yet ready
    retVal = snd_pcm_readi(m.handle, (char*)m.buffer.data(), m.frames);
    if(retVal < 0){
      throw std::string("microphone read failed: ") + snd_strerror(retVal);
    }

    //Next, calculate mean and stdev for rescaling and centering signals
    std::vector<std::pair<float, float> > l = meansAndStdDevs(m.buffer);
    //Find the loudness of the loudest channel
    float loudness = l[0].second;
    int loudest = 0;
    for(int i=1; i<l.size(); i++){
      if(l[i].second > loudness){
	loudness = l[i].second;
	loudest = i;
      }
    }

    //recenter(m.buffer, l);
    
    std::pair<float, float> delays[NUM_CHANNELS];
    for(int j=0; j < NUM_CHANNELS; j++){
      delays[j] = delay(m.buffer, 0, j, 2*SENSOR_SPACING_SAMPLES);
    }

    //LUT assumes that stream 0 is the primary stream, so the offets
    // user are 1, 2, and 3 (not 0)
    //Now do a LUT lookup
    std::vector<float> loc = {
      -delays[1].first,
      -delays[2].first,
      -delays[3].first
    };

    std::vector<float> entry = lut.get(loc);

    std::vector<float> cur_pt(entry.begin(), entry.begin()+3);
    static std::vector<float> last_pt = cur_pt;
        
    float d = dist(cur_pt, last_pt);
    //If lookup failed we get back 10.0f, so skip this data point
    if(cur_pt[0] < 2.0f){
      t.addPoint(cur_pt, loudness, frameNumber);    
    }

    s.putBuffer(m.buffer, loudness, loc);
    s.tickTo(frameNumber);
      
    frameNumber++;
    last_pt = cur_pt;
  }
  return 0;
}
