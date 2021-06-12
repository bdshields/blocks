/*
 * clock.c
 *
 *  Created on: 27 Mar 2021
 *      Author: brandon
 */

#include "colours.h"
#include "frame_buffer.h"
#include "frame_drv.h"
#include "image_util.h"
#include "pos.h"
#include "button.h"
#include "utils.h"
#include <time.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include "text.h"


/**
 *        #
 *       # #
 *     ##   #
 *     # # #
 *        #
 */
const raster_t clock_logo = {
    .x_max = 5,
    .y_max = 5,
    .image = {PX_CLEAR,PX_PURPL,PX_CLEAR,PX_CLEAR,PX_CLEAR,
            PX_PURPL,PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,
            PX_CLEAR,PX_ORANG,PX_CLEAR,PX_CLEAR,PX_CLEAR,
            PX_CLEAR,PX_CLEAR,PX_ORANG,PX_CLEAR,PX_CLEAR,
            PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_ORANG,PX_CLEAR,

    }
};

raster_t *clock_option(void)
{
    return &clock_logo;
}

void clock_run(uint16_t x, uint16_t y)
{
    user_input_t        button;
    raster_t            *screen;
    uint16_t            update_scr = 1;
    uint8_t             show_time = 1;
    systime             timeout;

    // CLOCK
    time_t              current_time;
    struct              tm broken_time;
    char                time_string[30];

    // TIMER
    uint32_t            seconds = 0;
    uint32_t            start_Seconds;
    uint8_t             entry_pos = 2;
    uint16_t             timer_values[] = {0,0,0,0,0,0};
    uint8_t             counting=0;


    uint8_t             mode = 0;

    screen = fb_allocate(x, y);
    fb_clear(screen);

    timeout = set_alarm(30000);

    while(1)
    {
        switch(mode)
        {
        case 0:
            // CLOCK
            current_time = time(NULL);
            localtime_r(&current_time, &broken_time);
            if((broken_time.tm_min == 59) && (broken_time.tm_sec == 54))
            {
                timeout = set_alarm(11500);
                show_time = 1;
            }

            if(show_time)
            {
                fb_clear(screen);
                strftime(time_string, 30, "%H", &broken_time);
                text_print(screen, POS(0,3), PX_PURPL, time_string);
                strftime(time_string, 30, "%M", &broken_time);
                text_print(screen, POS(10,3), PX_PURPL, time_string);
                strftime(time_string, 30, "%S", &broken_time);
                text_print(screen, POS(20,3), PX_PURPL, time_string);

                text_print(screen, POS(7,2), PX_ORANG, ":");
                text_print(screen, POS(17,2), PX_ORANG, ":");

                update_scr = 1;
            }
            if (alarm_expired(timeout))
            {
                cancel_alarm(&timeout);
                frame_drv_standby();
                show_time = 0;
            }
            switch(button.button)
            {
            case bu_select:
                show_time = 1;
                break;
            case bu_a:
                timeout = set_alarm(30000);
                show_time = 1;
                break;
            }
            break;
        case 1:
            // TIMER ENTRY
            fb_clear(screen);

            if(counting == 0)
            {
                snprintf(time_string, 30, "%03d:%02d", seconds / 600, (seconds/10) % 60);
                text_print(screen, POS(0,3), PX_PURPL, time_string);
                text_print(screen, POS(entry_pos*5,4), PX_YELLO, "_");
            }
            else
            {
                float count_ratio = (float)seconds / (float)start_Seconds;
                float angle = 0;
                float final_angle = 2*M_PI * count_ratio;
                float inc_x;
                float inc_y;
                float cur_x;
                float cur_y;
                pixel_t colour = PX_YELLO;
                do{
                    inc_x = sinf(angle);
                    inc_y = -cosf(angle);
                    cur_x = x/2;
                    cur_y = y/2;
                    do
                    {
                        // draw line from center to edge
                        fb_get_pixel(screen, roundf(cur_x), roundf(cur_y))[0] = colour;
                        cur_x += inc_x;
                        cur_y += inc_y;
                    }while(pos_out_raster(screen, POS(roundf(cur_x), roundf(cur_y))) == 0);
                    angle += 0.04;
                    if (angle > final_angle)
                    {
                        colour = PX_CYAN_;
                    }
                }while(angle < 2*M_PI);

                inc_x = sinf(final_angle);
                inc_y = -cosf(final_angle);
                cur_x = x/2;
                cur_y = y/2;
                do
                {
                    // draw line from center to edge
                    fb_get_pixel(screen, roundf(cur_x), roundf(cur_y))[0] = PX_DIMGREEN;
                    cur_x += inc_x;
                    cur_y += inc_y;
                }while(pos_out_raster(screen, POS(roundf(cur_x), roundf(cur_y))) == 0);

                snprintf(time_string, 30, "%03d:%02d", seconds / 600, (seconds/10) % 60);
                text_print(screen, POS(0,3), PX_PURPL, time_string);
            }

            update_scr = 1;

            switch(button.button)
            {
            case bu_a:
                if(counting == 0)
                {
                    timeout = set_alarm(100);
                    start_Seconds = seconds;
                    counting = 1;
                }
                else if(counting == 1)
                {
                    cancel_alarm(&timeout);
                    counting = 0;
                    show_time = 1;
                }
                break;
            case bu_left:
                entry_pos = entry_pos == 0 ? 0 : entry_pos - 1;
                entry_pos = entry_pos == 3 ? 2 : entry_pos;
                break;
            case bu_right:
                entry_pos = entry_pos == 5 ? 5 : entry_pos + 1;
                entry_pos = entry_pos == 3 ? 4 : entry_pos;
                break;
            case bu_up:
                timer_values[entry_pos] = timer_values[entry_pos] == 9 ? 0 : timer_values[entry_pos] + 1;
                show_time = 1;
                break;
            case bu_down:
                timer_values[entry_pos] = timer_values[entry_pos] == 0 ? 9 : timer_values[entry_pos] - 1;
                show_time = 1;
                break;
            case bu_select:
                cancel_alarm(&timeout);
                counting = 0;
                show_time = 1;
                break;
            }
            if(show_time && (counting == 0))
            {
                show_time = 0;
                seconds = timer_values[0] * 60000;
                seconds += timer_values[1] * 6000;
                seconds += timer_values[2] * 600;
                seconds += timer_values[4] * 100;
                seconds += timer_values[5] * 10;
                timeout = set_alarm(300000);
            }
            if (alarm_expired(timeout))
            {
                if(counting)
                {
                    if (seconds > 0)
                    {
                        timeout = set_alarm(100);
                        seconds-- ;
                    }
                    else
                    {
                        // revert to clock after 5 minutes
                        counting = 0;
                        show_time = 1;
                    }
                }
                else
                {
                    timeout = set_alarm(30000);
                    show_time = 1;
                    mode = 0;
                }
            }
            break;
        }

        button = in_get_bu();
        switch(button.button)
        {
        case bu_select:
            mode = mode >= 1 ? 0 : mode + 1;
            break;
        case bu_start:
            goto exit;

        }
        if(update_scr)
        {
        	update_scr = 0;
        	frame_drv_render(screen);
        }
        frame_sleep(100);
    }
exit:
	fb_destroy(screen);

}
