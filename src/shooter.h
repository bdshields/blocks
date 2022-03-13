/*
 * shooter.h
 *
 *  Created on: 13 Mar 2022
 *      Author: brandon
 */

#ifndef SRC_SHOOTER_H_
#define SRC_SHOOTER_H_

#include <stdint.h>
#include "frame_buffer.h"

raster_t *shooter_option(void);
void shooter_run(uint16_t x, uint16_t y);



#endif /* SRC_SHOOTER_H_ */
