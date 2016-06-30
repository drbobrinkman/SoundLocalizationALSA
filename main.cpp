#include <iostream>

/* Use the newer ALSA API */
#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>

int main() {
  int rc;
  snd_pcm_t *handle;
  snd_pcm_hw_params_t *params;
  unsigned int val, val2;
  int dir;
  snd_pcm_uframes_t frames;

  /* Open PCM device for playback. */
  rc = snd_pcm_open(&handle, "hw:1,0",
		    SND_PCM_STREAM_CAPTURE, 0);
  if (rc < 0) {
    fprintf(stderr,
	    "unable to open pcm device: %s\n",
	    snd_strerror(rc));
    exit(1);
  }

  /* Allocate a hardware parameters object. */
  snd_pcm_hw_params_alloca(&params);

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

  frames = 32;
  snd_pcm_hw_params_set_period_size_near(handle, params,
					 &frames, &dir);
  /* Write the parameters to the driver */
  rc = snd_pcm_hw_params(handle, params);
  if (rc < 0) {
    fprintf(stderr,
	    "unable to set hw parameters: %s\n",
	    snd_strerror(rc));
    exit(1);
  }

  /* Display information about the PCM interface */

  printf("PCM handle name = '%s'\n",
	 snd_pcm_name(handle));

  printf("PCM state = %s\n",
	 snd_pcm_state_name(snd_pcm_state(handle)));

  snd_pcm_hw_params_get_access(params,
			       (snd_pcm_access_t *) &val);
  printf("access type = %s\n",
	 snd_pcm_access_name((snd_pcm_access_t)val));

  /*  snd_pcm_hw_params_get_format(params, &val);
  printf("format = '%s' (%s)\n",
	 snd_pcm_format_name((snd_pcm_format_t)val),
	 snd_pcm_format_description(
				    (snd_pcm_format_t)val));

  snd_pcm_hw_params_get_subformat(params,
				  (snd_pcm_subformat_t *)&val);
  printf("subformat = '%s' (%s)\n",
	 snd_pcm_subformat_name((snd_pcm_subformat_t)val),
	 snd_pcm_subformat_description(
				       (snd_pcm_subformat_t)val));
  */
  snd_pcm_hw_params_get_channels(params, &val);
  printf("channels = %d\n", val);

  snd_pcm_hw_params_get_rate(params, &val, &dir);
  printf("rate = %d bps\n", val);

  snd_pcm_hw_params_get_period_time(params,
				    &val, &dir);
  printf("period time = %d us\n", val);

  snd_pcm_hw_params_get_period_size(params,
				    &frames, &dir);
  printf("period size = %d frames\n", (int)frames);

  snd_pcm_hw_params_get_buffer_time(params,
				    &val, &dir);
  printf("buffer time = %d us\n", val);

  snd_pcm_hw_params_get_buffer_size(params,
				    (snd_pcm_uframes_t *) &val);
  printf("buffer size = %d frames\n", val);

  snd_pcm_hw_params_get_periods(params, &val, &dir);
  printf("periods per buffer = %d frames\n", val);

  snd_pcm_hw_params_get_rate_numden(params,
				    &val, &val2);
  printf("exact rate = %d/%d bps\n", val, val2);

  val = snd_pcm_hw_params_get_sbits(params);
  printf("significant bits = %d\n", val);

  /*  snd_pcm_hw_params_get_tick_time(params,
				  &val, &dir);
  printf("tick time = %d us\n", val);
  */
  val = snd_pcm_hw_params_is_batch(params);
  printf("is batch = %d\n", val);

  val = snd_pcm_hw_params_is_block_transfer(params);
  printf("is block transfer = %d\n", val);

  val = snd_pcm_hw_params_is_double(params);
  printf("is double = %d\n", val);

  val = snd_pcm_hw_params_is_half_duplex(params);
  printf("is half duplex = %d\n", val);

  val = snd_pcm_hw_params_is_joint_duplex(params);
  printf("is joint duplex = %d\n", val);

  val = snd_pcm_hw_params_can_overrange(params);
  printf("can overrange = %d\n", val);

  val = snd_pcm_hw_params_can_mmap_sample_resolution(params);
  printf("can mmap = %d\n", val);

  val = snd_pcm_hw_params_can_pause(params);
  printf("can pause = %d\n", val);

  val = snd_pcm_hw_params_can_resume(params);
  printf("can resume = %d\n", val);

  val = snd_pcm_hw_params_can_sync_start(params);
  printf("can sync start = %d\n", val);


  /*Get frame size after init */
  snd_pcm_hw_params_get_period_size(params,
				    &frames, &dir);
  int size = frames*8; /* 2 bytes/samples, 4 channels */
  char* buffer = (char *)malloc(size);

  /* loop for 5 seconds */
  snd_pcm_hw_params_get_period_time(params,
				    &val, &dir);
  long loops = 5000000 / val;

  while (loops > 0) {
    loops--;
    rc = snd_pcm_readi(handle, buffer, frames);
    if (rc == -EPIPE) {
      /* EPIPE means overrun */
      fprintf(stderr, "overrun occurred\n");
      snd_pcm_prepare(handle);
    } else if (rc < 0) {
      fprintf(stderr,
	      "error from read: %s\n",
	      snd_strerror(rc));
    } else if (rc != (int)frames) {
      fprintf(stderr, "short read, read %d frames\n", rc);
    }
    //rc = write(1, buffer, size);
    //if (rc != size)
    //  fprintf(stderr,
    //      "short write: wrote %d bytes\n", rc);
  }

  snd_pcm_drain(handle);
  snd_pcm_close(handle);
  free(buffer);




  
  return 0;
}
