/*
 * oscilloscope.h
 *
 *  Created on: 29 Aug 2020
 *      Author: brandon
 */

#ifndef SRC_SPECTROGRAM_H_
#define SRC_SPECTROGRAM_H_


#include "frame_buffer.h"

raster_t *specgram_option(void);
void specgram_run(uint16_t x, uint16_t y);



#endif /* SRC_OSCILLOSCOPE_H_ */
