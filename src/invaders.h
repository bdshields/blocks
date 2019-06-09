/*
 * invaders.h
 *
 *  Created on: 15 May 2019
 *      Author: brandon
 */

#ifndef SRC_INVADERS_H_
#define SRC_INVADERS_H_

#include <stdint.h>
#include "frame_buffer.h"

raster_t *invaders_option(void);
void invaders_run(uint16_t x, uint16_t y);


#endif /* SRC_INVADERS_H_ */
