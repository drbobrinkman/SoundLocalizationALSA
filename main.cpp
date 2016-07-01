#include <iostream>

#include "microphone.h"

int main() {
  Microphone m;
  std::cout << m.bufferSize << std::endl;
  
  return 0;
}
