/*
 * clock.c
 *
 *  Created on: 27 Mar 2021
 *      Author: brandon
 */

#include "colours.h"
#include "frame_buffer.h"
#include "frame_drv.h"
#include "pos.h"
#include "button.h"
#include "utils.h"
#include <time.h>
#include <stddef.h>
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
	user_input_t    button;
	raster_t        *screen;
	uint16_t    update_scr=1;
	uint8_t         show_time=1;
    systime     timeout;

	time_t      current_time;
	struct tm   broken_time;
	char        time_string[30];

	screen = fb_allocate(x, y);
	fb_clear(screen);

	timeout = set_alarm(30000);

    while(1)
    {
        current_time = time(NULL);
        localtime_r(&current_time, &broken_time);
        if(((broken_time.tm_min == 59) && (broken_time.tm_sec > 55))
                || ((broken_time.tm_min == 0) && (broken_time.tm_sec < 5))
                || show_time)
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

        button = in_get_bu();
        switch(button.button)
        {
        case bu_a:
        case bu_b:
            timeout = set_alarm(30000);
            show_time = 1;
            break;
        case bu_start:
            goto exit;

        }
        if(update_scr)
        {
        	update_scr = 0;
        	frame_drv_render(screen);
        }
        if (alarm_expired(timeout))
        {
            frame_drv_standby();
            show_time = 0;
        }
        frame_sleep(100);
    }
exit:
	fb_destroy(screen);

}
