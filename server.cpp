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

#include "server.h"
#include "constants.h"
#include "soundProcessing.h"

/**
 * TODO: If I was a good person, we would possibly separate concerns
 *  here ... I've merged the handler with the server, not confident
 *  this is the best design.
 **/

#include <thread>
#include <mutex>
#include <fstream>
#include <streambuf>

using namespace boost::network;

std::mutex g_buffer_mutex;

void Server::operator() (http_server::request const &request,
		   http_server::response &response) {
  std::string command = destination(request);

  if(command.find("exit") == 1){
    //If I was a good person I might put a mutex on this, but
    // in a race condition we will just go around one more time
    running = false;
  } else if(command.find("sounds.json") == 1){
    std::lock_guard<std::mutex> guard(g_buffer_mutex);

    std::string response_str = "{\n";
    response_str += "    \"current_frame\": ";
    response_str += std::to_string(frameNumber);
    response_str += ",\n";
    
    response_str += "    \"sounds\": [";
    bool prev_entry = false;

    std::vector<Trackable> sounds
      = trck.getSoundsSince(frameNumberLastSentData);
    frameNumberLastSentData = frameNumber;
    
    for(int i=0; i < sounds.size(); i++){
      if(sounds[i].loudness < SILENCE_LOUDNESS) continue;

      if(prev_entry){
	response_str += "    }, ";
      }
      prev_entry = true;
      
      response_str += "{\n";

      response_str += "        \"location\": [";
      for(int j=0; j < 3; j++){
	response_str += std::to_string(sounds[i].location[j]);
	if(j < 2){
	  response_str += ", ";
	}
      }
      response_str += "],\n";

      response_str += "        \"first_frame\": ";
      response_str += std::to_string(sounds[i].firstFrame);
      response_str += ",\n";

      response_str += "        \"last_frame\": ";
      response_str += std::to_string(sounds[i].lastFrame);
      response_str += ",\n";

      response_str += "        \"loudness\": ";
      response_str += std::to_string(sounds[i].loudness);
      response_str += "\n";
    }
    if(prev_entry){
      response_str += "    }";
    }
    response_str += "]\n}\n";
    
    response = http_server::response::stock_reply
      (http_server::response::ok, response_str);

    http_server::response_header content_header;
    content_header.name = "Content-Type";
    content_header.value = "application/json";
    response.headers.push_back(content_header);
  } else if(command.find("tracker.html") == 1) {
    std::ifstream infile("tracker.html");
    std::string response_str((std::istreambuf_iterator<char>(infile)),
		    std::istreambuf_iterator<char>());
    
    response = http_server::response::stock_reply
      (http_server::response::ok, response_str);
  } else if(command.find("tracker.js") == 1) {
    std::ifstream infile("tracker.js");
    std::string response_str((std::istreambuf_iterator<char>(infile)),
		    std::istreambuf_iterator<char>());
    
    response = http_server::response::stock_reply
      (http_server::response::ok, response_str);

    http_server::response_header content_header;
    content_header.name = "Content-Type";
    content_header.value = "application/javascript";
    response.headers.push_back(content_header);
  } else {
    std::lock_guard<std::mutex> guard(g_buffer_mutex);
    
    std::string response_str
      = "<svg  xmlns=\"http://www.w3.org/2000/svg\" width=\"";
    response_str += std::to_string(4*(int)(0.5f + 16000.0f/TARGET_FRAME_RATE));
    response_str += "\" height=\"800\">\n";

    std::vector<std::string> colors = {"red", "green", "blue", "black"};

    if(loudness < 1.0f) loudness = 1.0f;

    if(buffer.size() > 0){
      for(int i=0; i<colors.size(); i++){
	response_str += "  <polyline points=\"";
	int offset = 0;
	if(i != 0){
	  offset = offsets[i-1];
	}
	int x = 0;
	for(int j=i; j < buffer.size(); j += 4){
	  int16_t val = buffer[j];
	  response_str += std::to_string(x + 4*offset) + ","
	    + std::to_string(50 + (val/(4*loudness))*50) + " ";
	  
	  x += 4;
	}
	response_str += "\" style=\"fill:none;stroke:";
	response_str += colors[i];
	response_str += ";stroke-width:1\" />\n";
	response_str += "<text x=\"0\" y=\"30\" style=\"black\">";
	response_str += std::to_string(loudness);
	response_str += "</text>\n";
      }
    }

    int corr_x = 100;
    int corr_y = 300;
    if(buffer.size() > 0){
      for(int i=0; i<colors.size(); i++){
	auto autocorr = xcorr(buffer,
			      i, i, 2*SENSOR_SPACING_SAMPLES);
	float max = 1.0f;
	for(int j=0; j < autocorr.size(); j++){
	  float val = std::abs(autocorr[j].second);
	  max = std::max(max, val);
	}
	response_str += "  <polyline points=\"";

	int x = 0;
	for(int j=0; j < autocorr.size(); j++){
	  std::pair<float, float> val = autocorr[j];
	  response_str += std::to_string(corr_x+3*val.first) + ","
	    + std::to_string(corr_y-50*(val.second/max)) + " "; //invert y axis
	}
	response_str += "\" style=\"fill:none;stroke:";
	response_str += colors[i];
	response_str += ";stroke-width:1\" />\n";
      }
    }

    response_str += "<polyline points=\"";
    response_str += std::to_string(corr_x-100) + "," + std::to_string(corr_y)
      + " " + std::to_string(corr_x+100) + "," + std::to_string(corr_y);
    response_str += "\" style=\"fill:none;stroke:";
    response_str += "black";
    response_str += ";stroke-width:1\" />\n";

    response_str += "<polyline points=\"";
    response_str += std::to_string(corr_x) + "," + std::to_string(corr_y - 50)
      + " " + std::to_string(corr_x) + "," + std::to_string(corr_y + 50);
    response_str += "\" style=\"fill:none;stroke:";
    response_str += "black";
    response_str += ";stroke-width:1\" />\n";

    if(buffer.size() > 0){
      for(int ch1=0; ch1<colors.size(); ch1++){
	for(int ch2=0; ch2 < colors.size(); ch2++){
	  auto autocorr = xcorr(buffer,
				ch1, ch2, 2*SENSOR_SPACING_SAMPLES);
	  float max = 1.0f;
	  for(int j=0; j < autocorr.size(); j++){
	    float val = std::abs(autocorr[j].second);
	    max = std::max(max, val);
	  }
	  response_str += "  <polyline points=\"";

	  int x = 0;
	  for(int j=0; j < autocorr.size(); j++){
	    std::pair<float, float> val = autocorr[j];
	    response_str += std::to_string(ch1*200 + corr_x+3*val.first) + ","
	      + std::to_string(ch2*100 + corr_y-50*(val.second/max)) + " ";
	  }
	  response_str += "\" style=\"fill:none;stroke:";
	  response_str += "black";
	  response_str += ";stroke-width:1\" />\n";

	  response_str += "<polyline points=\"";
	  response_str += std::to_string(ch1*200 + corr_x-100) + ","
	    + std::to_string(ch2*100 + corr_y)
	    + " " + std::to_string(ch1*200 + corr_x+100) + ","
	    + std::to_string(ch2*100 + corr_y);
	  response_str += "\" style=\"fill:none;stroke:";
	  response_str += "black";
	  response_str += ";stroke-width:1\" />\n";
	  
	  response_str += "<polyline points=\"";
	  response_str += std::to_string(ch1*200 + corr_x) + ","
	    + std::to_string(ch2*100 + corr_y - 50)
	    + " " + std::to_string(ch1*200 + corr_x) + ","
	    + std::to_string(ch2*100 + corr_y + 50);
	  response_str += "\" style=\"fill:none;stroke:";
	  response_str += "black";
	  response_str += ";stroke-width:1\" />\n";

	  response_str += "<polyline points=\"";
	  response_str += std::to_string(ch1*200 + corr_x + SENSOR_SPACING_SAMPLES*3) + ","
	    + std::to_string(ch2*100 + corr_y - 50)
	    + " " + std::to_string(ch1*200 + corr_x + SENSOR_SPACING_SAMPLES*3) + ","
	    + std::to_string(ch2*100 + corr_y + 50);
	  response_str += "\" style=\"fill:none;stroke:";
	  response_str += "gray";
	  response_str += ";stroke-width:1\" />\n";

	  response_str += "<polyline points=\"";
	  response_str += std::to_string(ch1*200 + corr_x - SENSOR_SPACING_SAMPLES*3) + ","
	    + std::to_string(ch2*100 + corr_y - 50)
	    + " " + std::to_string(ch1*200 + corr_x - SENSOR_SPACING_SAMPLES*3) + ","
	    + std::to_string(ch2*100 + corr_y + 50);
	  response_str += "\" style=\"fill:none;stroke:";
	  response_str += "gray";
	  response_str += ";stroke-width:1\" />\n";

	  
	  std::pair<float, float> delay_ = delay(buffer,
						 ch1, ch2,
						 MAX_OFFSET);
	  response_str += "<circle cx=\""
	    + std::to_string(ch1*200 + corr_x + delay_.first*3) +
	    "\" cy=\"" + std::to_string(ch2*100 + corr_y)
	    + "\" fill=\"green\" r=\"2\"/>\n";
	  
	  response_str += "<text x=\"" + std::to_string(ch1*200 + corr_x) +
	    "\" y=\"" + std::to_string(ch2*100 + corr_y)
	    + "\" fill=\"black\">" + std::to_string(ch1) + std::to_string(ch2)
	    + "</text>\n";
	}
      }
    }
    
    response_str += "<circle cx=\"200\" cy=\"198\" r=\"2\" fill=\"black\"/>\n";
    response_str += "<circle cx=\"202\" cy=\"200\" r=\"2\" fill=\"black\"/>\n";
    response_str += "<circle cx=\"198\" cy=\"200\" r=\"2\" fill=\"black\"/>\n";

    std::vector<Trackable> sounds
      = trck.getSoundsSince(frameNumberLastSentData);

    /*    for(int i=0; i < sounds.size(); i++){
      if(sounds[i].loudness < SILENCE_LOUDNESS) continue;
      
      response_str += "<circle cx=\"";
      response_str += std::to_string(200 + sounds[i].location[0]*50);
      response_str += "\" cy=\"";
      response_str += std::to_string(200 - sounds[i].location[1]*50);
      response_str += "\" r=\"";
      response_str += std::to_string(sounds[i].loudness/200);
      response_str += "\" fill=\"blue\"/>\n";
      }*/
    
    response_str += "</svg>\n";
    
    response = http_server::response::stock_reply
      (http_server::response::ok, response_str);

    http_server::response_header content_header;
    content_header.name = "Content-Type";
    content_header.value = "image/svg+xml";
    response.headers.push_back(content_header);
  }
}

bool Server::isRunning(){
  //TODO: Mutex? Maybe I don't care about race conditions for this one
  return running;
}

void Server::log(http_server::string_type const &info) {
  std::cerr << "ERROR: " << info << '\n';
}

void Server::run(){
  if(p_server){
    p_server->run();
  } else {
    throw std::string("Tried to run server without server instance");
  }
}

Server::Server(Tracker& itrk) : trck(itrk)
{
  static http_server::options options_(*this);
  static http_server server_(options_.address("0.0.0.0").port("8000"));
  p_server = &server_;

  static std::thread t_(&Server::run, this);
  t_.detach();
}

Server::~Server(){
}

void Server::putBuffer(std::vector<int16_t> &ibuffer, float iloudness,
		       std::vector<float> ioffsets,
			//std::vector<Trackable> isounds,
		       unsigned long iframeNum){
  std::lock_guard<std::mutex> guard(g_buffer_mutex);

  offsets = ioffsets;
  buffer = ibuffer;
  loudness = iloudness;
  //sounds = isounds;
  frameNumber = iframeNum;
}
