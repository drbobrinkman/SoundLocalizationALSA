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
LIBS=-lasound -lpthread -lboost_system -lboost_thread -lcppnetlib-uri -lcppnetlib-server-parsers -lcppnetlib-client-connections
OBJ = main.o microphone.o soundProcessing.o locationlut.o spherepoints.o server.o tracker.o updateServer.o utils.o

default: sla

main.o: main.cpp microphone.h locationlut.h constants.h server.h \
 tracker.h soundProcessing.h updateServer.h utils.h
	$(CPP) -c -o $@ $< $(CFLAGS) $(PRODFLAGS)

microphone.o: microphone.cpp microphone.h constants.h
	$(CPP) -c -o $@ $< $(CFLAGS) $(PRODFLAGS)

soundProcessing.o: soundProcessing.cpp soundProcessing.h constants.h
	$(CPP) -c -o $@ $< $(CFLAGS) $(PRODFLAGS)

locationlut.o: locationlut.cpp locationlut.h constants.h \
 spherepoints.h utils.h
	$(CPP) -c -o $@ $< $(CFLAGS) $(PRODFLAGS)

spherepoints.o: spherepoints.cpp spherepoints.h
	$(CPP) -c -o $@ $< $(CFLAGS) $(PRODFLAGS)

server.o: server.cpp server.h tracker.h constants.h soundProcessing.h
	$(CPP) -c -o $@ $< $(CFLAGS) $(PRODFLAGS)

tracker.o: tracker.cpp tracker.h constants.h utils.h
	$(CPP) -c -o $@ $< $(CFLAGS) $(PRODFLAGS)

updateServer.o: updateServer.cpp updateServer.h
	$(CPP) -c -o $@ $< $(CFLAGS) $(PRODFLAGS)

utils.o: utils.cpp utils.h
	$(CPP) -c -o $@ $< $(CFLAGS) $(PRODFLAGS)

sla: $(OBJ)
	$(CPP) -o $@ $^ $(CFLAGS) $(PRODFLAGS) $(LIBS)

clean:
	rm -f *.o *~ core
