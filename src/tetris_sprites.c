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
    .x_max = 4,
    .y_max = 4,
    .image = {PX_CLEAR,PX_YELLO,PX_CLEAR,PX_CLEAR,
              PX_CLEAR,PX_YELLO,PX_YELLO,PX_CLEAR,
              PX_CLEAR,PX_YELLO,PX_CLEAR,PX_CLEAR,
              PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR
    }
};

/**
 *      #
 *      #
 *      ##
 */
const raster_t tetris_2 = {
    .x_max = 4,
    .y_max = 4,
    .image = {PX_CLEAR,PX_PURPL,PX_CLEAR,PX_CLEAR,
              PX_CLEAR,PX_PURPL,PX_CLEAR,PX_CLEAR,
              PX_CLEAR,PX_PURPL,PX_PURPL,PX_CLEAR,
              PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR
    }
};

/**
 *       #
 *       #
 *      ##
 */
const raster_t tetris_3 = {
    .x_max = 4,
    .y_max = 4,
    .image = {PX_CLEAR,PX_CLEAR,PX_ORANG,PX_CLEAR,
            PX_CLEAR,PX_CLEAR,PX_ORANG,PX_CLEAR,
            PX_CLEAR,PX_ORANG,PX_ORANG,PX_CLEAR,
            PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR

    }
};

/**
 *     #
 *     ##
 *      #
 */
const raster_t tetris_4 = {
    .x_max = 4,
    .y_max = 4,
    .image = {PX_CLEAR,PX_CYAN_,PX_CLEAR,PX_CLEAR,
              PX_CLEAR,PX_CYAN_,PX_CYAN_,PX_CLEAR,
              PX_CLEAR,PX_CLEAR,PX_CYAN_,PX_CLEAR,
              PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR

    }
};

/**
 *       #
 *      ##
 *      #
 */
const raster_t tetris_5 = {
    .x_max = 4,
    .y_max = 4,
    .image = {PX_CLEAR,PX_CLEAR,PX_GREEN,PX_CLEAR,
              PX_CLEAR,PX_GREEN,PX_GREEN,PX_CLEAR,
              PX_CLEAR,PX_GREEN,PX_CLEAR,PX_CLEAR,
              PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR
    }
};

/**
 *     ##
 *     ##
 */
const raster_t tetris_6 = {
    .x_max = 4,
    .y_max = 4,
    .image = {PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,
            PX_CLEAR,PX_RED__,PX_RED__,PX_CLEAR,
            PX_CLEAR,PX_RED__,PX_RED__,PX_CLEAR,
            PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR
    }
};

/**
 *      #
 *      #
 *      #
 *      #
 */
const raster_t tetris_7 = {
    .x_max = 4,
    .y_max = 4,
    .image = {PX_CLEAR,PX_BLUE_,PX_CLEAR,PX_CLEAR,
              PX_CLEAR,PX_BLUE_,PX_CLEAR,PX_CLEAR,
              PX_CLEAR,PX_BLUE_,PX_CLEAR,PX_CLEAR,
              PX_CLEAR,PX_BLUE_,PX_CLEAR,PX_CLEAR
    }
};


