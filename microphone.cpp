#include <iostream>

#include "microphone.h"

Microphone::Microphone(int iTargetFrameRate) {
  targetFrameRate = iTargetFrameRate;
  
  int rc;
  unsigned int val, val2;
  int dir;
  snd_pcm_uframes_t frames;

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
  bufferSize = frames*8; /* 2 bytes/samples, 4 channels */
  buffer = (char *)malloc(bufferSize);
}

Microphone::~Microphone(){
  snd_pcm_close(handle);
  free(buffer);
}
