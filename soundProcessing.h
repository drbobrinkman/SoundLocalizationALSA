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

#include <queue>
#include <utility>
#include <vector>

std::vector<std::pair<float, float> >
  meansAndStdDevs(char* buffer, unsigned int frames);
float diffWithOffset(char* buffer, unsigned int frames,
		     unsigned int ch1, unsigned int ch2,
		     int offset);
float dotWithOffset(char* buffer, unsigned int frames,
		     unsigned int ch1, unsigned int ch2,
		     int offset);
std::vector<std::pair<float, float> > xcorr(char* buffer, unsigned int frames,
					    unsigned int ch1, unsigned int ch2,
					    int range);
std::pair<float, float> delay(char* buffer, unsigned int frames,
	    unsigned int ch1, unsigned int ch2,
	    int range);

float diffFourway(char* buffer, unsigned int frames,
		  int offset[3]);

void findTopNOffsets(char* buffer, unsigned int frames,
	      unsigned int ch1, unsigned int ch2, int n,
	      std::priority_queue<std::pair<float, int> >& outList);
void recenter(char* buffer, unsigned int frames,
			std::vector<std::pair<float, float> > stats);
float dist(std::vector<float> a, std::vector<float> b);
std::vector<float> lerp(std::vector<float> a, std::vector<float> b, float amt);
