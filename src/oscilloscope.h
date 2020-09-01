/*
 * oscilloscope.h
 *
 *  Created on: 29 Aug 2020
 *      Author: brandon
 */

#ifndef SRC_OSCILLOSCOPE_H_
#define SRC_OSCILLOSCOPE_H_


#include "frame_buffer.h"

raster_t *osci_option(void);
void osci_run(uint16_t x, uint16_t y);



#endif /* SRC_OSCILLOSCOPE_H_ */
