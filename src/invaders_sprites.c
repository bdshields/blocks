/*
 * invaders_sprites.c
 *
 *  Created on: 15 May 2019
 *      Author: brandon
 */

#include "frame_buffer.h"
#include "colours.h"




/**
 *      # #
 *     #####
 *    ##   ##
 *      # #
 */
const raster_t invader_logo = {
    .x_max = 5,
    .y_max = 5,
    .image = {PX_GREEN,PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_GREEN,
              PX_CLEAR,PX_GREEN,PX_CLEAR,PX_GREEN,PX_CLEAR,
              PX_GREEN,PX_GREEN,PX_GREEN,PX_GREEN,PX_GREEN,
              PX_GREEN,PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_GREEN,
              PX_CLEAR,PX_GREEN,PX_CLEAR,PX_GREEN,PX_CLEAR,

    }
};

/**
 *      #
 */
const raster_t invader_alien = {
    .x_max = 1,
    .y_max = 1,
    .image = {PX_GREEN,
    }
};

/**
 *      ##
 */
const raster_t invader_ship = {
    .x_max = 2,
    .y_max = 1,
    .image = {PX_RED__,PX_RED__,
    }
};

/**
 *      #
 */
const raster_t invader_cannon = {
    .x_max = 1,
    .y_max = 1,
    .image = {PX_BLUE_,
    }
};

/**
 *      #
 */
const raster_t invader_bomb = {
    .x_max = 1,
    .y_max = 1,
    .image = {PX_YELLO,
    }
};

