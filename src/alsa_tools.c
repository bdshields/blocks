/*
 * alsa_tools.c
 *
 *  Created on: 29 Aug 2020
 *      Author: brandon
 */

#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

#include "alsa_tools.h"

#include "debug.h"
#define DEBUG_LEVEL DBG_DEBUG

snd_pcm_t *alsa_open(char *dev)
{
    int err;
    unsigned int rate = ALSA_SRATE;
    snd_pcm_t *capture_handle;
    snd_pcm_hw_params_t *hw_params;
    snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;

    if ((err = snd_pcm_open (&capture_handle, dev, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
        DEBUG("cannot open audio device %s (%s)\n",
              dev,
               snd_strerror (err));
      exit (1);
    }

    DEBUG("audio interface opened\n");

    if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
        DEBUG("cannot allocate hardware parameter structure (%s)\n",
               snd_strerror (err));
      exit (1);
    }

    DEBUG("hw_params allocated\n");

    if ((err = snd_pcm_hw_params_any (capture_handle, hw_params)) < 0) {
        DEBUG("cannot initialize hardware parameter structure (%s)\n",
               snd_strerror (err));
      exit (1);
    }

    DEBUG("hw_params initialized\n");

    if ((err = snd_pcm_hw_params_set_access (capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        DEBUG("cannot set access type (%s)\n",
               snd_strerror (err));
      exit (1);
    }

    DEBUG("hw_params access setted\n");

    if ((err = snd_pcm_hw_params_set_format (capture_handle, hw_params, format)) < 0) {
        DEBUG("cannot set sample format (%s)\n",
               snd_strerror (err));
      exit (1);
    }

    DEBUG("hw_params format set\n");

#if 0
    if ((err = snd_pcm_hw_params_set_rate_resample (capture_handle, hw_params, rate)) < 0) {
      fprintf (stderr, "cannot set sample rate (%s)\n",
               snd_strerror (err));
      exit (1);
    }

#else
    if ((err = snd_pcm_hw_params_set_rate_near (capture_handle, hw_params, &rate, 0)) < 0) {
        DEBUG("cannot set sample rate (%s)\n",
               snd_strerror (err));
      exit (1);
    }
#endif
    DEBUG("hw_params rate set (%d)\n",rate);

    if ((err = snd_pcm_hw_params_set_channels (capture_handle, hw_params, ALSA_CHAN)) < 0) {
        DEBUG("cannot set channel count (%s)\n",
               snd_strerror (err));
      exit (1);
    }

    DEBUG("hw_params channels set\n");

    if ((err = snd_pcm_hw_params (capture_handle, hw_params)) < 0) {
        DEBUG("cannot set parameters (%s)\n",
               snd_strerror (err));
      exit (1);
    }

    DEBUG("hw_params set\n");

    snd_pcm_hw_params_free (hw_params);

    DEBUG("hw_params freed\n");

    if ((err = snd_pcm_prepare (capture_handle)) < 0) {
        DEBUG("cannot prepare audio interface for use (%s)\n",
               snd_strerror (err));
      exit (1);
    }

    DEBUG("audio interface prepared\n");

    return capture_handle;
}

int alsa_read(snd_pcm_t *capture_handle, char *buffer, int buffer_frames)
{
    int err;
    if ((err = snd_pcm_readi (capture_handle, buffer, buffer_frames)) != buffer_frames) {
        DEBUG("read from audio interface failed (%s)\n",
               snd_strerror (err));
    }
    return err;
}


void alsa_close(snd_pcm_t *capture_handle)
{
    snd_pcm_close (capture_handle);
    DEBUG("audio interface closed\n");

}
