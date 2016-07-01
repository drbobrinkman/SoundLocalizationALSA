#pragma once

/* Use the newer ALSA API */
#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>

//TODO: If I was a good person, I'd make this a singleton
class Microphone {
 public:
  Microphone(int iTargetFrameRate = 60);
  ~Microphone();
  
  int targetFrameRate;
  
  snd_pcm_t *handle;
  snd_pcm_hw_params_t *params;
  snd_pcm_uframes_t frames;

  char* buffer;
  int bufferSize;
};
