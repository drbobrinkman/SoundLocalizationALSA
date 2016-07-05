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

/**
 * Portions of this code come from http://www.linuxjournal.com/node/6735/print
 * Tranter, Jeff. "Introduction to Sound Programming with ALSA." Linux Journal,
 *  Sep 30, 2004.
 **/

#pragma once

/* Use the newer ALSA API */
#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>

class Microphone {
 public:
  static Microphone& getInstance(){
    static Microphone instance;
    return instance;
  }
  
 private:
  //This is to make sure we don't try to make Microphones using new and
  // delete
  Microphone();
  ~Microphone();

 public:
  //This is to make sure we don't forget to declare our variables
  // as references
  Microphone(Microphone const&) = delete;
  void operator=(Microphone const&) = delete;
  
  static const int targetFrameRate = 60;
  
  snd_pcm_t *handle;
  snd_pcm_hw_params_t *params;
  snd_pcm_uframes_t frames;
  const unsigned int BYTES_PER_FRAME = 8;
  
  char* buffer;
  //Note: Buffer size should be frames*8, because we are using
  // 16-bit format with 4 channels
};
