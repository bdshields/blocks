/*
 * oscilloscope.h
 *
 *  Created on: 29 Aug 2020
 *      Author: brandon
 */

#ifndef SRC_SPECTRUM_H_
#define SRC_SPECTRUM_H_


#include "frame_buffer.h"

raster_t *spec_option(void);
void spec_run(uint16_t x, uint16_t y);



#endif /* SRC_OSCILLOSCOPE_H_ */
