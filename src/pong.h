/*
 * pong.h
 *
 *  Created on: 23 May 2019
 *      Author: brandon
 */

#ifndef SRC_PONG_H_
#define SRC_PONG_H_

#include <stdint.h>
#include "frame_buffer.h"


void pong_run(uint16_t x, uint16_t y);
raster_t *pong_option(void);


#endif /* SRC_PONG_H_ */
