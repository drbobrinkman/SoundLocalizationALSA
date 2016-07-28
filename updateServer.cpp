/** \file updateServer.cpp
 * This is a kludgey solution to service discovery. 
 * 
 * \bug Uses shelvar.com to 
 * store the IP address of the server, so that it can be looked up by the
 * client. If the client were a Mac we could use hostname.local, but haven't
 * found a quick and easy solution for Windows 10 devices. This could be
 * exploited.
 *
 * \author Bo Brinkman <dr.bo.brinkman@gmail.com>
 * \date 2016-07-27
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

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <sstream>

#include <boost/network/protocol/http/client.hpp>

namespace http = boost::network::http;

#include "updateServer.h"

/*! Executes another program, returning whatever it prints as a string.
 * \param cmd a command to be run
 * \return Whatever the command prints to standard output (stderr is not 
 * included)
 *
 * \note Code from http://stackoverflow.com/questions/478898/how-to-execute-a-command-and-get-output-of-command-within-c-using-posix
 *
 */
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
