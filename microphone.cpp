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
#include <iostream>

#include "microphone.h"

Microphone::Microphone() {
  int rc;
  unsigned int val, val2;
  int dir;

  /* Open PCM device for playback. */
  rc = snd_pcm_open(&handle, "hw:1,0",
		    SND_PCM_STREAM_CAPTURE, 0);
  if (rc < 0) {
    throw std::string("unable to open pcm device: ") + snd_strerror(rc);
  }

  /* Allocate a hardware parameters object. */
  snd_pcm_hw_params_alloca(&params); //TODO: Is there a way to deallocate
  // this? I can'd find it in the ALSA examples...

  /* Fill it in with default values. */
  snd_pcm_hw_params_any(handle, params);

  /* Set the desired hardware parameters. */

  /* Interleaved mode */
  snd_pcm_hw_params_set_access(handle, params,
			       SND_PCM_ACCESS_RW_INTERLEAVED);

  /* Signed 16-bit little-endian format */
  snd_pcm_hw_params_set_format(handle, params,
			       SND_PCM_FORMAT_S16_LE);

  snd_pcm_hw_params_set_channels(handle, params, 4);

  val = 16000;
  snd_pcm_hw_params_set_rate_near(handle,
				  params, &val, &dir);

  frames = (int)(0.5 + (double)val/targetFrameRate);
  snd_pcm_hw_params_set_period_size_near(handle, params,
					 &frames, &dir);
  /* Write the parameters to the driver */
  rc = snd_pcm_hw_params(handle, params);
  if (rc < 0) {
    snd_pcm_close(handle);
    throw std::string("unable to set hw parameters: ") + snd_strerror(rc);
  }

  /*Get frame size after init */
  snd_pcm_hw_params_get_period_size(params,
				    &frames, &dir);
  int size = frames*BYTES_PER_FRAME; /* 2 bytes/samples, 4 channels */
  buffer.resize(size, 0);
}

Microphone::~Microphone(){
  snd_pcm_close(handle);
}
