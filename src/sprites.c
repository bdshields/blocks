/*
 * bitmaps.c
 *
 *  Created on: 28 Apr. 2019
 *      Author: brandon
 */

#include "sprites.h"

#include "frame_buffer.h"

const raster_t happy = {
    .x_max = 8,
    .y_max = 8,
    .image = {PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,
              PX_CLEAR,PX_BLUE_,PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_BLUE_,PX_CLEAR,
              PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_BLUE_,PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,
              PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_BLUE_,PX_BLUE_,PX_CLEAR,PX_CLEAR,PX_CLEAR,
              PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,
              PX_CLEAR,PX_BLUE_,PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_BLUE_,PX_CLEAR,
              PX_CLEAR,PX_CLEAR,PX_BLUE_,PX_BLUE_,PX_BLUE_,PX_BLUE_,PX_CLEAR,PX_CLEAR,
              PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,
    }
};

const raster_t sad = {
    .x_max = 8,
    .y_max = 8,
    .image = {PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,
              PX_CLEAR,PX_RED__,PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_RED__,PX_CLEAR,
              PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_RED__,PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,
              PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_RED__,PX_RED__,PX_CLEAR,PX_CLEAR,PX_CLEAR,
              PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,
              PX_CLEAR,PX_CLEAR,PX_RED__,PX_RED__,PX_RED__,PX_RED__,PX_CLEAR,PX_CLEAR,
              PX_CLEAR,PX_RED__,PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_RED__,PX_CLEAR,
              PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,
    }
};


const raster_t cursor = {
    .x_max = 1,
    .y_max = 1,
    .image = {PX_PURPL}
};
