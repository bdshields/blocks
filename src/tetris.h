/*
 * tetris.h
 *
 *  Created on: 5 May 2019
 *      Author: brandon
 */

#ifndef SRC_TETRIS_H_
#define SRC_TETRIS_H_

#include <stdint.h>
#include "frame_buffer.h"

raster_t *tetris_option(void);

void tetris_run(uint16_t x, uint16_t y);

#endif /* SRC_TETRIS_H_ */
