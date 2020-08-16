/*
 * paint.c
 *
 *  Created on: 10 Sep. 2019
 *      Author: brandon
 */

#include "frame_drv.h"
#include "frame_buffer.h"
#include "colours.h"
#include "input.h"
#include "image_util.h"
#include "pos.h"

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


raster_t *paint_option(void)
{
    return &paint_logo;
}

void paint_run(uint16_t x, uint16_t y)
{
    raster_t        *canvas;
    user_input_t    button;
    pos_t           pos_brush={0,0};
    uint16_t    update_scr=1;

    canvas = fb_allocate(x, y);
    clear_raster(canvas);
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
        case bu_start:
            goto exit;
        }
        if(update_scr)
        {
            paste_sprite(canvas, &paint_prush, pos_brush);
            frame_drv_render(canvas);
            update_scr = 0;
        }
    }
exit:
    fb_destroy(canvas);

}


