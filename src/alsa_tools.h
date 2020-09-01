/*
 * alsa_tools.h
 *
 *  Created on: 29 Aug 2020
 *      Author: brandon
 */

#ifndef SRC_ALSA_TOOLS_H_
#define SRC_ALSA_TOOLS_H_

#include <alsa/asoundlib.h>


#define DEVICE "hw:1,0"

#define ALSA_SRATE 48000
#define ALSA_CHAN  2


snd_pcm_t *alsa_open(char *dev);
int alsa_read(snd_pcm_t *capture_handle, char *buffer, int buffer_frames);
void alsa_close(snd_pcm_t *capture_handle);


#endif /* SRC_ALSA_TOOLS_H_ */
