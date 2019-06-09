/*
 * screen_saver.h
 *
 *  Created on: 19 May 2019
 *      Author: brandon
 */

#ifndef SRC_SAVER_RIPPLES_H_
#define SRC_SAVER_RIPPLES_H_

#include <stdint.h>
#include "frame_buffer.h"

raster_t *ripples_option(void);
void ripples_run(uint16_t x, uint16_t y);


#endif /* SRC_SAVER_RIPPLES_H_ */
