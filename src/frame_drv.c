/*
 * frame_drv.c
 *
 *  Created on: 12 May 2019
 *      Author: brandon
 */

#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "frame_drv.h"
#include "frame_buffer.h"
#include "utils.h"


#ifdef WS2811_LIB
#include <ws2811.h>

#define WS2811_MAX_POWER 5
#define GPIO_PIN 12

#define WS2811_MAX_WIDTH 15
#define WS2811_STRIP_WIDTH 15
#define WS2811_MAX_HEIGHT 30



ws2811_t ws2811_strip =
{
    .freq = WS2811_TARGET_FREQ,
    .dmanum = 10,
    .channel =
    {
        [0] =
        {
            .gpionum = GPIO_PIN,
            .count = WS2811_MAX_WIDTH * WS2811_MAX_HEIGHT,
            .invert = 0,
            .brightness = 64,
            .strip_type = WS2811_STRIP_GRB,
        },
        [1] =
        {
            .gpionum = 0,
            .count = 0,
            .invert = 0,
            .brightness = 0,
        },
    },
};


#endif

drv_type frame_drv_type = dr_none;
disp_state frame_drv_state = ds_none;

int frame_drv_init(int x, int y, drv_type type)
{
    int result=0;
    switch(type)
    {
    case dr_term:
        // Use escape sequence to set window size
        printf("\x1b[8;%d;%dt",y+5, (x*2)+2);
        frame_drv_type = type;
        frame_drv_state = ds_active;
        result = 1;
        break;
#ifdef WS2811_LIB
    case dr_ws2811:
    {
        ws2811_return_t ret;
        system("echo 1 > /sys/class/gpio/gpio23/value");
        frame_sleep(1000);
        if ((ret = ws2811_init(&ws2811_strip)) != WS2811_SUCCESS)
        {
            fprintf(stderr, "ws2811_init failed: %s\n", ws2811_get_return_t_str(ret));
            return 0;
        }
        else
        {
            frame_drv_type = dr_ws2811;
            frame_drv_state = ds_active;
            result = 1;
        }
        break;
    }
#endif
    }

    return result;
}


int frame_drv_render(raster_t * raster)
{
    pixel_t    *cur_pixel;

    frame_drv_wake();

    switch(frame_drv_type)
    {
    case dr_term:
    {
        uint16_t    x;
        uint16_t    y;
        char       temp_buffer[1000];
        // Use escape sequence to move cursor to top of frame ready for next frame.
        printf("\x1b[0;0f");
        // Move down a line
        printf("\x1b[1B");
        // Print a boarder around our display
        for(x=0; x<(raster->x_max*2)+2; x++)
        {
            temp_buffer[x]='-';
        }
        temp_buffer[x]=0;
        printf("%s\n",temp_buffer);

        cur_pixel = raster->image;
        for(y=0; y<raster->y_max; y++)
        {
            printf("|");
            for(x=0; x<raster->x_max; x++)
            {
                // use escape sequence to set background colour
                if(cur_pixel->flags)
                {
                    printf("\x1b[48;2;%d;%d;%dm %X",cur_pixel->red,cur_pixel->green,cur_pixel->blue,cur_pixel->flags);
                }
                else
                {
                    printf("\x1b[48;2;%d;%d;%dm  ",cur_pixel->red,cur_pixel->green,cur_pixel->blue);
                }
                cur_pixel++;
            }
            printf("\x1b[48;2;0;0;0m");
            printf("|\n");
        }
        // Print a boarder around our display
        for(x=0; x<(raster->x_max*2)+2; x++)
        {
            temp_buffer[x]='-';
        }
        temp_buffer[x]=0;
        printf("%s\n",temp_buffer);
        break;
    }
#ifdef WS2811_LIB
    case dr_ws2811:
    {
        int16_t    strip_start_x_dir[]={1,-1,1};
        int16_t    strip_dir;
        int16_t    raster_x;
        int16_t    raster_y;
        int16_t    min_x;     // Min value inclusive
        int16_t    max_x;     // Max value inclusive
        uint16_t    led_index;
        int16_t     step_x=1;
        int16_t     step_y=1;
        int16_t     strip_index;

        // Coordinates of raster that map to LED 0
        raster_x = 0;
        raster_y = 0;

        // Direction to step through raster
        step_x=strip_start_x_dir[0];
        step_y=1;

        max_x = (1*WS2811_STRIP_WIDTH) -1;
        min_x = (0*WS2811_STRIP_WIDTH);

        strip_dir = 1;
        strip_index=1;

        for(led_index=0; led_index<ws2811_strip.channel[0].count; led_index++)
        {
#if 1
            // copy data from raster to LEDs
            cur_pixel = fb_get_pixel(raster,raster_x,raster_y);
            ws2811_strip.channel[0].leds[led_index] = (uint32_t)cur_pixel->red << 16
                                                                | (uint32_t)cur_pixel->green << 8
                                                                | (uint32_t)cur_pixel->blue << 0;
            raster_x += step_x;
            if(step_x == 1)
            {
                if(raster_x > max_x)
                {
                    raster_y += step_y;
                    step_x = -1;
                    raster_x += step_x;
                }
            }
            else if(step_x == -1)
            {
                if(raster_x < min_x)
                {
                    raster_y += step_y;
                    step_x = 1;
                    raster_x += step_x;
                }
            }
            if(step_y == 1)
            {
                if(raster_y == WS2811_MAX_HEIGHT)
                {
                    strip_index++;
                    step_y = -1;
                    min_x += (WS2811_STRIP_WIDTH * strip_dir);
                    max_x += (WS2811_STRIP_WIDTH * strip_dir);
                    raster_y += step_y;
                    raster_x = strip_start_x_dir[strip_index] == 1 ? min_x : max_x;
                    step_x = strip_start_x_dir[strip_index];
                }
            }
            else if(step_y == -1)
            {
                if(raster_y < 0)
                {
                    strip_index++;
                    step_y = 1;
                    min_x += (WS2811_STRIP_WIDTH * strip_dir);
                    max_x += (WS2811_STRIP_WIDTH * strip_dir);
                    raster_y += step_y;
                    raster_x = strip_start_x_dir[strip_index] == 1 ? min_x : max_x;
                    step_x = strip_start_x_dir[strip_index];
                }
            }

#else
            // copy data from raster to LEDs
            ws2811_strip.channel[0].leds[y] = (uint32_t)cur_pixel->red << 16
                                                                | (uint32_t)cur_pixel->green << 8
                                                                | (uint32_t)cur_pixel->blue << 0;
            x ++;
            if(x == raster->y_max)
            {
                x = 0;
                // adjust pointer ready for next scan in oposite direction
                cur_pixel ++;
                state ^= 0x01;
            }
            else
            {
                if(state & 0x01)
                {
                    // downwards
                    cur_pixel-= raster->x_max;
                }
                else
                {
                    // upwards
                    cur_pixel += raster->x_max;
                }
            }
#endif
        }
        ws2811_render(&ws2811_strip);
        break;
    }
#endif
    }
    return 1;
}

/**
 * Put display to sleep
 */
void frame_drv_standby()
{
    switch(frame_drv_type)
    {
#ifdef WS2811_LIB
    case dr_ws2811:
        if (frame_drv_state == ds_active)
        {
            ws2811_fini(&ws2811_strip);
            system("echo 0 > /sys/class/gpio/gpio23/value");
            frame_drv_state = ds_sleep;
        }
        break;
#endif
    }
}

void frame_drv_wake()
{
    switch(frame_drv_type)
    {
#ifdef WS2811_LIB
    case dr_ws2811:
        if (frame_drv_state == ds_sleep)
        {
            frame_drv_state = ds_active;
            system("echo 1 > /sys/class/gpio/gpio23/value");
            frame_sleep(1000);
            ws2811_init(&ws2811_strip);
            //ws2811_render(&ws2811_strip);
        }
        break;
#endif
    }
}

int frame_drv_shutdown()
{
    switch(frame_drv_type)
    {
    case dr_term:

        break;
#ifdef WS2811_LIB
    case dr_ws2811:
        memset(ws2811_strip.channel[0].leds, 0, ws2811_strip.channel[0].count * sizeof(ws2811_led_t));
        ws2811_render(&ws2811_strip);
        ws2811_fini(&ws2811_strip);
        break;
#endif
    }
    return 1;
}
