# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
# 
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

CPP=g++
CFLAGS=-std=c++14
DBGFLAGS=-g -O0
PRODFLAGS=-O3
LIBS=-lasound -lpthread -lboost_system -lcppnetlib-uri -lcppnetlib-server-parsers -lcppnetlib-client-connections
DEPS = microphone.h soundProcessing.h tracking.h locationlut.h spherepoints.h server.h
OBJ = main.o microphone.o soundProcessing.o tracking.o locationlut.o spherepoints.o server.o

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) $(PRODFLAGS)

sla: $(OBJ)
	g++ -o $@ $^ $(CFLAGS) $(PRODFLAGS) $(LIBS)

clean:
	rm -f *.o *~ core

default: sla
