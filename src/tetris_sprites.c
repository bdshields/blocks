/*
 * tetris_sprites.c
 *
 *  Created on: 5 May 2019
 *      Author: brandon
 */

#include "frame_buffer.h"
#include "colours.h"

/**
 *      #
 *      ##
 *      #
 */
const raster_t tetris_1 = {
    .x_max = 2,
    .y_max = 3,
    .center = ORIGIN(0.5,1),
    .image = {PX_YELLO,PX_CLEAR,
              PX_YELLO,PX_YELLO,
              PX_YELLO,PX_CLEAR
    }
};

/**
 *      #
 *      #
 *      ##
 */
const raster_t tetris_2 = {
    .x_max = 2,
    .y_max = 3,
    .center = ORIGIN(0.5,1),
    .image = {PX_PURPL,PX_CLEAR,
              PX_PURPL,PX_CLEAR,
              PX_PURPL,PX_PURPL
    }
};

/**
 *       #
 *       #
 *      ##
 */
const raster_t tetris_3 = {
    .x_max = 2,
    .y_max = 3,
    .center = ORIGIN(0.5,1),
    .image = {PX_CLEAR,PX_ORANG,
            PX_CLEAR,PX_ORANG,
            PX_ORANG,PX_ORANG

    }
};

/**
 *     #
 *     ##
 *      #
 */
const raster_t tetris_4 = {
    .x_max = 2,
    .y_max = 3,
    .center = ORIGIN(0.5,1),
    .image = {PX_CYAN_,PX_CLEAR,
              PX_CYAN_,PX_CYAN_,
              PX_CLEAR,PX_CYAN_

    }
};

/**
 *       #
 *      ##
 *      #
 */
const raster_t tetris_5 = {
    .x_max = 2,
    .y_max = 3,
    .center = ORIGIN(0.5,1),
    .image = {PX_CLEAR,PX_GREEN,
              PX_GREEN,PX_GREEN,
              PX_GREEN,PX_CLEAR
    }
};

/**
 *     ##
 *     ##
 */
const raster_t tetris_6 = {
    .x_max = 2,
    .y_max = 2,
    .center = ORIGIN(0.5,0.5),
    .image = {PX_RED__,PX_RED__,
            PX_RED__,PX_RED__
    }
};

/**
 *      #
 *      #
 *      #
 *      #
 */
const raster_t tetris_7 = {
    .x_max = 1,
    .y_max = 4,
    .center = ORIGIN(0,1.5),
    .image = {PX_BLUE_,
              PX_BLUE_,
              PX_BLUE_,
              PX_BLUE_    }
};


