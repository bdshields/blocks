/*
 * pong_sprites.c
 *
 *  Created on: 23 May 2019
 *      Author: brandon
 */

#include "frame_buffer.h"
#include "colours.h"


const raster_t pong_logo = {
    .x_max = 5,
    .y_max = 5,
    .image = {
            PX_YELLO,PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,
            PX_YELLO,PX_CLEAR,PX_CLEAR,PX_PURPL,PX_YELLO,
            PX_YELLO,PX_CLEAR,PX_PURPL,PX_CLEAR,PX_YELLO,
            PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_YELLO,
            PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR
    }
};


const raster_t pong_paddle = {
        .x_max = 4,
        .y_max = 1,
        .image = {
                PX_PURPL,PX_YELLO,PX_YELLO,PX_PURPL
        }
};
