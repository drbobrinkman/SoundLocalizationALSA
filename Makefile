CPP=g++
CFLAGS=-std=c++14
LIBS=-lasound
DEPS = microphone.h soundProcessing.h tracking.h
OBJ = main.o microphone.o soundProcessing.o tracking.o

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

sla: $(OBJ)
	g++ -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	rm -f *.o *~ core

default: sla
