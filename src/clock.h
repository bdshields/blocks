/*
 * paint.h
 *
 *  Created on: 10 Sep. 2019
 *      Author: brandon
 */

#ifndef CLOCK_H_
#define CLOCK_H_


#include "frame_buffer.h"

raster_t *clock_option(void);
void clock_run(uint16_t x, uint16_t y);

#endif /* CLOCK_H_ */
