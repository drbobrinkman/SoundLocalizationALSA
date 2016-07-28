/** \file microphone.h
 * Manages setup and teardown of the microphone, via ALSA.
 * 
 * \note Portions of this code come from 
 * http://www.linuxjournal.com/node/6735/print
 * Tranter, Jeff. "Introduction to Sound Programming with ALSA." Linux Journal,
 * Sep 30, 2004.
 *
 * \author Bo Brinkman <dr.bo.brinkman@gmail.com>
 * \date 2016-07-28
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

#pragma once

/*! Use the newer ALSA API */
#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>

#include <vector>
#include <cstdint>

/*! Class to manage an ALSA microphone 
 *
 * \note Singleton, with lazy initialization. (Meyers style singleton) 
 *
 * \bug We are not doing any encapsulation here, all of the variables
 * are public. The main purpose of this class is to provide resource
 * acquisition and cleanup.
 */
class Microphone {
 public:
  /*! Return the singleton instance. */
  static Microphone& getInstance(){
    static Microphone instance;
    return instance;
  }
  
 private:
  //ctor and dtor are private to encourage correct usage of singleton
  Microphone();
  ~Microphone();

 public:
  /*! Copy ctor deleted so that we don't accidentally make a copy */
  Microphone(Microphone const&) = delete;
  /*! Copy assignment deleted so that we don't accidentally make a copy */
  void operator=(Microphone const&) = delete;

  /*! ALSA handle for the microphone */
  snd_pcm_t *handle;
  /*! ALSA object for hw parameters */
  snd_pcm_hw_params_t *params;
  /*! Number of frames we want per call to snd_pcm_readi */
  snd_pcm_uframes_t frames;
  /*! Number of channels in the signal */
  unsigned int channels;
  /*! Rate of the signal, in samples per second */
  unsigned int rate;

  /*! Buffer for storing the most recently read sound clip */
  std::vector<int16_t> buffer;
};
