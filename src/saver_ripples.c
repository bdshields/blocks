/*
 * screen_saver.c
 *
 *  Created on: 19 May 2019
 *      Author: brandon
 */
#include <stdlib.h>

#include "frame_buffer.h"
#include "frame_drv.h"
#include "image_util.h"
#include "colours.h"
#include "button.h"
#include "pos.h"
#include "utils.h"

const raster_t ripples_logo = {
    .x_max = 5,
    .y_max = 5,
    .image = {
            PX_WATERBLUE ,PX_CLEAR,PX_CLEAR,    PX_CLEAR,    PX_CLEAR,
              PX_WATERBLUE,PX_CLEAR,PX_CLEAR,    PX_CLEAR,    PX_CLEAR,
              PX_CLEAR ,PX_WATERBLUE,PX_WATERBLUE,PX_WATERBLUE,PX_CLEAR,
              PX_CLEAR,PX_CLEAR,    PX_CLEAR,    PX_CLEAR,    PX_WATERBLUE,
              PX_CLEAR,PX_CLEAR,    PX_CLEAR,    PX_CLEAR,    PX_WATERBLUE

    }
};

typedef enum colour_method_e{
    green_blue,
    blue_red,
    green_blue_purple,
    method_end
}colour_method_t;

raster_t *ripples_option(void)
{
    return &ripples_logo;
}

#define VALUE_MASK 0x03

#define V(_a)  ((value_index+(_a)) & VALUE_MASK)

#define ATTEMPT 1

#define DECAY   (1)  // Decay at boundary

#define VISCOSITY (0.99)  // Max of 1.0


#define DROPLET_DURATION 3
#define RIPPLE_FPS 30

pixel_t colourize(colour_method_t method, int16_t intensity);

void ripples_run(uint16_t x, uint16_t y)
{
    raster_t *screen_area;
    int16_t     *value[4];
    uint16_t    value_index=0;

    user_input_t    button;
    systime     update_tmr;
    uint16_t    index_x;
    uint16_t    index_y;
    pos_t       droplet_pos;
    uint16_t    droplet_timer;

    colour_method_t method = green_blue;
    float           viscosity = 1;

    int32_t total;
    // allocate frame buffer

    screen_area = fb_allocate(x, y);

    value[0] = malloc(sizeof(int16_t)*x*y);
    value[1] = malloc(sizeof(int16_t)*x*y);
    value[2] = malloc(sizeof(int16_t)*x*y);
    value[3] = malloc(sizeof(int16_t)*x*y);

    for(index_y=0; index_y<(x*y); index_y++)
    {
        value[0][index_y] = 0;
        value[1][index_y] = 0;
        value[2][index_y] = 0;
        value[3][index_y] = 0;
    }

    droplet_pos = (pos_t){rand()%x, rand()%y};
    droplet_timer =DROPLET_DURATION;

    update_tmr = set_alarm(1000/RIPPLE_FPS);
    while(1)
    {
        if(alarm_expired(update_tmr))
        {
            update_tmr = set_alarm(1000/RIPPLE_FPS);
            // Update droplet
            if(droplet_timer > 0)
            {
                value[V(-2)][droplet_pos.y*x + droplet_pos.x]=-30000;
                droplet_timer--;
            }

            // Update propagation
            for(index_y=0; index_y<y; index_y++)
            {
                for(index_x=0; index_x<x; index_x++)
                {
                    int16_t neighbours = 4;
                    int16_t left, right, up, down, center[3];
                    float decay = 1;


                    center[0] = value[V(-3)][index_y*x + index_x];
                    center[1] = value[V(-2)][index_y*x + index_x];
                    center[2] = value[V(-1)][index_y*x + index_x];
                    if(index_x>0)
                    {
                        left = value[V(-1)][index_y*x + index_x - 1];
                    }
                    else
                    {
                        left = 0;
                        neighbours --;
                        decay = DECAY;
                    }
                    if(index_x<(x-1))
                    {
                        right = value[V(-1)][index_y*x + index_x + 1];
                    }
                    else
                    {
                        right = 0;
                        neighbours --;
                        decay = DECAY;
                    }
                    if(index_y>0)
                    {
                        up = value[V(-1)][(index_y-1)*x + index_x];
                    }
                    else
                    {
                        up =0;
                        neighbours --;
                        decay = DECAY;
                    }
                    if(index_y<(y-1))
                    {
                        down = value[V(-1)][(index_y+1)*x + index_x];
                    }
                    else
                    {
                        down = 0;
                        neighbours --;
                        decay = DECAY;
                    }

                    total = decay * ((((1.0+viscosity)/4.0)*(left + right + up + down)) - (viscosity * value[V(-2)][index_y*x+ index_x]));

                    if(total > 30000)
                    {
                        total = 30000;
                    }
                    else if(total < -30000)
                    {
                        total = -30000;
                    }
                    value[V(0)][index_y*x + index_x] = total;
                }
            }
            fb_clear(screen_area);
            for(index_y = 0; index_y < (x*y); index_y++)
            {
                screen_area->image[index_y] = colourize(method, value[V(0)][index_y]);
            }
            // Illuminate the droplet
            if(droplet_timer > 0)
            {
                fb_get_pixel(screen_area, droplet_pos.x, droplet_pos.y)->red = 255;
            }

            frame_drv_render(screen_area);
            value_index++;

        }
        else
        {
            frame_sleep(10);
        }

        button = in_get_bu();
        switch(button.button)
        {
        case bu_up:
            if(viscosity < 1.02)
            {
                viscosity += 0.01;
            }
            break;
        case bu_down:
            if(viscosity > 0.85)
            {
                viscosity -= 0.01;
            }
            break;
        case bu_left:
            break;
        case bu_right:
            break;
        case bu_a:
            droplet_pos = (pos_t){rand()%x, rand()%y};
            droplet_timer =DROPLET_DURATION;
            break;
        case bu_b:
            method ++;
            if(method == method_end)
            {
                method = 0;
            }
            break;
        case bu_start:
            goto exit;
            break;
        }
    }
exit:
    fb_destroy(screen_area);

    free(value[0]);
    free(value[1]);
    free(value[2]);
    free(value[3]);
}

pixel_t colourize(colour_method_t method, int16_t intensity)
{
    int32_t value;
    pixel_t colours;
    int16_t red=0;
    int16_t green=0;
    int16_t blue=0;


    value = intensity;

    switch (method)
    {
    case green_blue:

        value += 0x7FFF; // 0 -> 2^16
        value >>= 7;  // 0 -> 512


        blue = value - 256 + 64;
        if(blue > 255)
        {
            blue = 255;
        }
        else if(blue < 0)
        {
            blue = 0;
        }
        green = -(value - 255 - 64);
        if(green > 255)
        {
            green = 255;
        }
        else if(green < 0)
        {
            green = 0;
        }

        break;
    case blue_red:

        value += 0x7FFF;
        value >>= 7;  // 0 - 512


        red = value - 256 + 64;
        if(red > 255)
        {
            red = 255;
        }
        else if(red < 0)
        {
            red = 0;
        }
        blue = -(value - 255 - 64);
        if(blue > 255)
        {
            blue = 255;
        }
        else if(blue < 0)
        {
            blue = 0;
        }

        break;
    case green_blue_purple:
        value += 0x7FFF; // 0 -> 2^16
        value >>= 6;  // 0 -> 1024

        red = value - 512 + 64;
        if(red > 255)
        {
            red = 255;
        }
        else if(red < 0)
        {
            red = 0;
        }

        blue = value - 256 + 64;
        if(blue > 255)
        {
            blue = 255;
        }
        else if(blue < 0)
        {
            blue = 0;
        }
        green = -(value - 255 - 64);
        if(green > 255)
        {
            green = 255;
        }
        else if(green < 0)
        {
            green = 0;
        }

        break;
    }
    colours.blue =  blue;
    colours.green = green;
    colours.red = red;
    colours.flags = R_VISIBLE;
    return colours;
}
