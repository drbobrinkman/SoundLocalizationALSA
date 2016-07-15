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

/**
 * TODO: If I was a good person, we would possibly separate concerns
 *  here ... I've merged the handler with the server, not confident
 *  this is the best design.
 **/

void Server::operator() (http_server::request const &request,
		   http_server::response &response) {
  response = http_server::response::stock_reply(
	         http_server::response::ok, "Hello, world!");
}

void Server::log(http_server::string_type const &info) {
  std::cerr << "ERROR: " << info << '\n';
}

void Server::run(){
  http_server::options options(*this);
  http_server server_(options.address("0.0.0.0")
		             .port("8000"));
  server_.run();
}

Server::Server() : t(&Server::run, this){
  t.detach();
}

Server::~Server(){
}
