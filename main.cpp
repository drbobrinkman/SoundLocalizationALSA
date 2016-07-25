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
#include "constants.h"
#include "tracker.h"

//Next 5 includes are for the exec() method
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <sstream>

#include <boost/network/protocol/http/client.hpp>

std::string exec(const char* cmd) {
  char buffer[128];
  std::string result = "";
  std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
  if (!pipe) throw std::runtime_error("popen() failed!");
  while (!feof(pipe.get())) {
    if (fgets(buffer, 128, pipe.get()) != NULL)
      result += buffer;
  }
  return result;
}

void updateIPDiscoveryServer(){
  //TODO: Using exec to call ifconfig is probably not the right way to
  // to do this
  std::string res = exec("/sbin/ifconfig");
  std::istringstream ifconfigin(res);
  std::string nibble;
  while(ifconfigin >> nibble, nibble != "wlan0"){
    //Do nothing, just throw it all away
  }
  while(ifconfigin >> nibble, nibble != "inet"){
        //Do nothing, just throw it all away
  }
  ifconfigin >> nibble;
  std::string address(nibble.begin() + 1 + nibble.find(":"),
		      nibble.end());
  std::cout << "Server running at: |" << address << "|" << std::endl;
  http::client client_;
  try {
    http::client::request request_("http://shelvar.com/ip.php?ip=" + address);
    //request_ << http::client::header("Conection", "close");
    
    http::client::response response_ = client_.get(request_);
    std::cout << "Discovery service update result: " << body(response_)
	      << std::endl;;
  } catch (std::exception& e) {
    // deal with exceptions here
  }
}

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

    recenterAndRescale(m.buffer.data(), m.frames, l);
    
    //The starting value was determined empirically
    static float background_loudness = 100.0f;

    float delays[4];
    for(int i=0; i < 4; i++){
      delays[i] = delay(m.buffer.data(), m.frames, loudest, i, MAX_OFFSET);
    }
    //LUT assumes that stream 0 is the primary stream, so the offets
    // user are 1, 2, and 3 (not 0)
    //Now do a LUT lookup
    std::vector<float> loc = {
      delays[0] - delays[1],
      delays[0] - delays[2],
      delays[0] - delays[3]
    };
    std::vector<float> entry = lut.get(loc);

    std::vector<float> cur_pt(entry.begin(), entry.begin()+3);
    static std::vector<float> last_pt = cur_pt;
        
    float d = dist(cur_pt, last_pt);
    //If lookup failed we get back 10.0f, so skip this data point
    if(cur_pt[0] < 2.0f){
      t.addPoint(cur_pt, loudness, frameNumber);    
    }
    s.putBuffer(m.buffer, loudness, loc, frameNumber);

    frameNumber++;
    last_pt = cur_pt;
  }
  return 0;
}
