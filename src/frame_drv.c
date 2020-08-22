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

#define WS2811_MAX_WIDTH 30
#define WS2811_MAX_HEIGHT 15



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
        struct scan_step{
            int repeat;
            int x_step;
            int y_step;
        };
        /**
         * Iterates through the LED list in sequence starting at 0
         * Multiple pre-fabricated LED pcb pannels are supported
         *
         * Raster dimensions
         * 0,0  == Top Left Corner
         */
        // Coordinates of raster that map to first LED in chain
        int16_t    raster_x = 0;
        int16_t    raster_y = 14;

        int16_t    step_index;
        int16_t    step_count;
        int16_t    counter;

        int16_t    led_index;

        // How to scan the raster
        const struct scan_step    scan_directions[]={
        {1,0,0},{14,0,-1},{1,1,0},{14,0,1},{1,1,0},
                {14,0,-1},{1,1,0},{14,0,1},{1,1,0},
                {14,0,-1},{1,1,0},{14,0,1},{1,1,0},
                {14,0,-1},{1,1,0},{14,0,1},{1,1,0},
                {14,0,-1},{1,1,0},{14,0,1},{1,1,0},
                {14,0,-1},{1,1,0},{14,0,1},{1,1,0},
                {14,0,-1},{1,1,0},{14,0,1},{1,1,0},
                {14,0,-1},{1,1,0},{14,0,1},{1,1,0},
                {14,0,-1},{1,1,0},{14,0,1},{1,1,0},
                {14,0,-1},{1,1,0},{14,0,1},{1,1,0},
                {14,0,-1},{1,1,0},{14,0,1},{1,1,0},
                {14,0,-1},{1,1,0},{14,0,1},{1,1,0},
                {14,0,-1},{1,1,0},{14,0,1},{1,1,0},
                {14,0,-1},{1,1,0},{14,0,1},{1,1,0},
                {14,0,-1},{1,1,0},{14,0,1}
        };

        step_count = sizeof(scan_directions)/sizeof(struct scan_step);

        step_index = 0;
        counter = scan_directions[step_index].repeat;

        for(led_index=0; led_index<ws2811_strip.channel[0].count; led_index++)
        {
            // copy data from raster to LEDs
            cur_pixel = fb_get_pixel(raster,raster_x,raster_y);
            ws2811_strip.channel[0].leds[led_index] = (uint32_t)cur_pixel->red << 16
                                                                | (uint32_t)cur_pixel->green << 8
                                                                | (uint32_t)cur_pixel->blue << 0;

            // Get next step
            counter --;
            if(counter == 0)
            {
                step_index ++;
                if(step_index >= step_count)
                {
                    break;
                }
                counter = scan_directions[step_index].repeat;
            }
            // Update raster coordinates
            raster_x += scan_directions[step_index].x_step;
            raster_y += scan_directions[step_index].y_step;

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
