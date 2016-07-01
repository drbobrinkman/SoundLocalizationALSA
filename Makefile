CPP=g++
CFLAGS=
LIBS=-lasound
DEPS = microphone.h
OBJ = main.o microphone.o

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

sla: $(OBJ)
	g++ -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	rm -f *.o *~ core

default: sla
