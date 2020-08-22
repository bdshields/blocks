/*
 * paint.c
 *
 *  Created on: 10 Sep. 2019
 *      Author: brandon
 */

#include "frame_drv.h"
#include "frame_buffer.h"
#include "colours.h"
#include "button.h"
#include "image_util.h"
#include "frame_buffer.h"
#include "pos.h"
#include "utils.h"

/**
 *        #
 *       # #
 *     ##   #
 *     # # #
 *        #
 */
const raster_t paint_logo = {
    .x_max = 5,
    .y_max = 5,
    .image = {PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_GREEN,PX_CLEAR,
              PX_CLEAR,PX_CLEAR,PX_GREEN,PX_CLEAR,PX_GREEN,
              PX_PURPL,PX_GREEN,PX_CLEAR,PX_CLEAR,PX_GREEN,
              PX_PURPL,PX_CLEAR,PX_GREEN,PX_GREEN,PX_GREEN,
              PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_GREEN,PX_CLEAR,

    }
};

const raster_t paint_prush = {
    .x_max = 1,
    .y_max = 1,
    .image = {PX_PURPL}
};

const raster_t paint_cursor = {
    .x_max = 1,
    .y_max = 1,
    .image = {PX_GREEN}
};

raster_t *paint_option(void)
{
    return &paint_logo;
}

void paint_run(uint16_t x, uint16_t y)
{
    raster_t        *canvas;
    raster_t        *screen;
    user_input_t    button;
    pos_t           pos_brush={0,0};
    uint16_t    update_scr=1;
    uint16_t    mode;
    uint16_t    erase = 0;

    canvas = fb_allocate(x, y);
    screen = fb_allocate(x, y);
    clear_raster(canvas);
    clear_raster(screen);

    mode = 0;
    while(1)
    {
        button = in_get_bu();
        switch(button.button)
        {
        case bu_up:
            pos_brush.y --;
            if(pos_out_raster(canvas, pos_brush))
            {
                pos_brush.y ++;
            }
            update_scr = 1;
            break;
        case bu_down:
            pos_brush.y ++;
            if(pos_out_raster(canvas, pos_brush))
            {
                pos_brush.y --;
            }
            update_scr = 1;
            break;
        case bu_left:
            pos_brush.x --;
            if(pos_out_raster(canvas, pos_brush))
            {
                pos_brush.x ++;
            }
            update_scr = 1;
            break;
        case bu_right:
            pos_brush.x ++;
            if(pos_out_raster(canvas, pos_brush))
            {
                pos_brush.x --;
            }
            update_scr = 1;
            break;
        case bu_a:
            // flip the mode
            mode = mode == 0;
            update_scr = 1;
            break;
        case bu_b:
            erase = 1;
            mode = 0;
            update_scr = 1;
            break;
        case bu_start:
            goto exit;
        }
        if(update_scr)
        {
            clear_raster(screen);
            if (mode == 1)
            {
                // brush down
                paste_sprite(canvas, &paint_prush, pos_brush);
                paste_sprite(screen, canvas, (pos_t){0,0});
            }
            else
            {
                if (erase == 1)
                {
                    // erase pixel at current location
                    fb_get_pixel(canvas,pos_brush.x,pos_brush.y)->flags &= ~R_VISIBLE;
                    paste_sprite(screen, canvas, (pos_t){0,0});
                    paste_sprite(screen, &paint_cursor, pos_brush);
                    erase = 0;
                }
                else
                {
                    //cursor
                    paste_sprite(screen, canvas, (pos_t){0,0});
                    paste_sprite(screen, &paint_cursor, pos_brush);
                }
            }
            frame_drv_render(screen);
            update_scr = 0;
        }
        else
        {
            frame_sleep(50);
        }
    }
exit:
    fb_destroy(canvas);

}


