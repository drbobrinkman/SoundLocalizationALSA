#include <iostream>

#include "microphone.h"

int main() {
  Microphone m;
  std::cout << (int)m.frames << std::endl;
  
  return 0;
}
