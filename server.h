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

#pragma once

#include <thread>
#include <vector>
#include <cstdint>

#include "tracker.h"

#include <boost/network/protocol/http/server.hpp>
namespace http = boost::network::http;

class Server;
typedef http::server<Server> http_server;

class Server {
 public:
  static Server& getInstance(Tracker& trk){
    static Server instance(trk);
    return instance;
  }
  
 private:
  //This is to make sure we don't try to make Servers using new and
  // delete
  Server(Tracker& itrck);
  ~Server();

  std::thread t;
  Tracker& trck;
  
  bool running=true;
  
 public:
  //This is to make sure we don't forget to declare our variables
  // as references
  Server(Server const&) = delete;
  void operator=(Server const&) = delete;

  bool isRunning();
  void putBuffer(std::vector<int16_t> &ibuffer, float iloudness,
		 std::vector<float> loc,
		 //std::vector<Trackable> isounds,
		 unsigned long iframeNum);

  std::vector<int16_t> buffer;
  std::vector<float> offsets;
  //std::vector<Trackable> sounds;
  float loudness;
  unsigned long frameNumber;
  unsigned long frameNumberLastSentData = -1;
  
  http_server::options options;
  http_server server_;
  
 public:
  void run();
	   
  void log(http_server::string_type const &info);
  void operator() (http_server::request const &request,
		   http_server::response &response);
};
