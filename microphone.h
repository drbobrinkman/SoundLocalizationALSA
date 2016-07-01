#pragma once

/* Use the newer ALSA API */
#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>

//TODO: If I was a good person, I'd make this a singleton
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
