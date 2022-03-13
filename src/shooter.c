/*
 * shooter.c
 *
 *  Created on: 13 March 2022
 *      Author: brandon
 */

#include "shooter.h"
#include <stdlib.h>
#include "utils.h"
#include "pos.h"
#include "button.h"
#include "image_util.h"
#include "frame_drv.h"
#include "colours.h"
#include "scoring.h"
#include "http_session.h"



enum shooter_state {
    ss_none,
    ss_start_delay,
    ss_run,
    ss_move_up,
    ss_move_down,
    ss_crash,
    ss_game_win
};

enum shooter_event {
    se_start,
    se_move_up,
    se_move_down,
    se_crash,
    se_scroll,
    se_timeout,
    se_none
};

const raster_t shooter_logo = {
    .x_max = 5,
    .y_max = 5,
    .image = {PX_LITEBLUE,PX_LITEBLUE,PX_LITEBLUE,PX_LITEBLUE,PX_LITEBLUE,
            PX_LITEBLUE,PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_LITEBLUE,
              PX_RED__,PX_CLEAR,PX_CLEAR,PX_GREEN,PX_CLEAR,
              PX_CLEAR,PX_GREEN,PX_GREEN,PX_GREEN,PX_GREEN,
              PX_GREEN,PX_GREEN,PX_GREEN,PX_GREEN,PX_GREEN,

    }
};


raster_t *shooter_option(void)
{
    return &shooter_logo;
}

typedef struct {
    int16_t ground_height;
    int16_t sky_depth;
}slice_t;
slice_t generate_terrain_slice(slice_t slice1, slice_t slice2, int height);

void generate_terrain(slice_t *terrain, uint16_t x, uint16_t y);

#define SHIP_XPOS 2

void shooter_run(uint16_t x, uint16_t y)
{
    raster_t *game_area;
    raster_t *terrain_area;
    user_input_t button;
    int       update_scr=0;
    systime     timeout;
    systime     scroll_timeout;
    int16_t     ship_pos = y/2;

    slice_t shooter_terrain[x];
    int first_slice = 0;


    enum shooter_state game_state = ss_none;
    enum shooter_event game_event = se_start;
    if(http_session_setPlayers(1) == 1)
    {
        http_session_setTeams(1);
        score_init("Shooter", 1,http_session_getTeamName(1));
    }
    else
    {
        score_init("Shooter", 1,"Player 1");
    }
    game_area = fb_allocate(x, y);
    terrain_area = fb_allocate(x, y);
    cancel_alarm(&timeout);
    cancel_alarm(&scroll_timeout);
    while(1)
    {
        // Process events
        enum shooter_event tempEvt = game_event;
        game_event = se_none;
        switch(tempEvt)
        {
        case se_start:
            ship_pos = y/2;
            first_slice = 0;
            generate_terrain(shooter_terrain, x, y);
            update_scr = 1;
            game_state = ss_start_delay;
            timeout = set_alarm(2000);
            break;
        case se_scroll:
            shooter_terrain[first_slice % x] = generate_terrain_slice(shooter_terrain[(first_slice + x - 2)%x],shooter_terrain[(first_slice + x - 1)%x], y);
            first_slice ++;
            first_slice %= x;
            game_state = ss_run;
            scroll_timeout = set_alarm(250);
            update_scr = 1;
            break;
        case se_move_up:
            if(game_state == ss_run)
            {
                ship_pos --;
                update_scr = 1;
            }
            break;
        case se_move_down:
            if(game_state == ss_run)
            {
                ship_pos ++;
                update_scr = 1;
            }
            break;
        case se_crash:
            game_state = ss_crash;
            cancel_alarm(&scroll_timeout);
            timeout = set_alarm(2000);
            break;
        case se_timeout:
            switch(game_state)
            {
            case ss_start_delay:
                game_state = ss_run;
                game_event = se_scroll;
                break;
            case ss_crash:
                game_state = ss_none;
                game_event=se_start;
                break;
            case ss_run:
                break;
            }
            break;
        case se_none:
            frame_sleep(10);
            break;
        }

        // Look for events
        button = in_get_bu();
        if(button.user == 1)
        {
            switch(button.button)
            {
            case bu_up:
                game_event = se_move_up;
                break;
            case bu_down:
                game_event = se_move_down;
                break;
            case bu_a:
                break;
            case bu_start:
                goto exit;
                break;
            }
        }
        else if(alarm_expired(timeout))
        {
            game_event = se_timeout;
            cancel_alarm(&timeout);
        }
        else if(alarm_expired(scroll_timeout))
        {
            game_event = se_scroll;
            cancel_alarm(&scroll_timeout);
        }
        if(update_scr)
        {
            int x_cntr;
            int y_cntr;
            fb_clear(game_area);
            fb_clear(terrain_area);
            // TERRAIN
            for(x_cntr=0; x_cntr< x; x_cntr++)
            {
                for(y_cntr=0; y_cntr< y; y_cntr++)
                {
                    if (y_cntr <= shooter_terrain[(first_slice + x_cntr) % x].ground_height)
                    {
                        *fb_get_pixel(terrain_area, x_cntr, y_cntr) = PX_LITEBLUE;
                    }
                    else if (y_cntr >= shooter_terrain[(first_slice + x_cntr) % x].sky_depth)
                    {
                        *fb_get_pixel(terrain_area, x_cntr, y_cntr) = PX_GREEN;
                    }
                }
            }
            // SHIP
            if(fb_get_pixel(terrain_area, SHIP_XPOS, ship_pos)->flags & R_VISIBLE)
            {
                game_event = se_crash;
                *fb_get_pixel(terrain_area, SHIP_XPOS, ship_pos) = PX_RED__;
            }
            else
            {
                *fb_get_pixel(terrain_area, SHIP_XPOS, ship_pos) = PX_RED__;
            }

            frame_drv_render(terrain_area);
            update_scr = 0;
        }
        else
        {
        }




    }
exit:
    score_save();
    fb_destroy(terrain_area);
    fb_destroy(game_area);
}


#define GAP  6

slice_t generate_terrain_slice(slice_t slice1, slice_t slice2, int height)
{
    slice_t new_slice;
    int16_t ground_slope = 0;
    int16_t sky_slope = 0;
    int16_t gap = GAP;

    ground_slope = slice2.ground_height - slice1.ground_height;
    sky_slope = slice2.sky_depth - slice1.sky_depth;
    new_slice = slice2;

    /**
     *  Slope change constraints
     *  Options:
     *      UP, Down, Flat
     *  Dynamics
     *      Prefer to maintain slope
     *      Prefer Slope instead of flat
     *
     *
     */
    if (ground_slope == 0)
    {
        if (rand() < (RAND_MAX / 100 * 20))
        {
            // Stay flat
            ground_slope = 0;
        }
        else if(rand() < (RAND_MAX / 100 * 50))
        {
            ground_slope = 1;
        }
        else
        {
            ground_slope = -1;
        }
    }
    else
    {
        if (rand() < (RAND_MAX / 100 * 60))
        {
            // no change
        }
        else if(rand() < (RAND_MAX / 100 * 60))
        {
            ground_slope = 0;
        }
        else
        {
            ground_slope *= -1;
        }
    }

    // Widen gap when slopy
    gap += (ground_slope * ground_slope);

    new_slice.ground_height += ground_slope;
    if(new_slice.ground_height < 0 )
    {
        // just reverse the slope
        new_slice.ground_height = 1;
    }
    else if(new_slice.ground_height > (height - gap - 1) )
    {
        // just reverse the slope
        new_slice.ground_height = height - gap - 1;
    }

    if (sky_slope == 0)
    {
        if (rand() < (RAND_MAX / 100 * 20))
        {
            // Stay flat
            sky_slope = 0;
        }
        else if(rand() < (RAND_MAX / 100 * 50))
        {
            sky_slope = 1;
        }
        else
        {
            sky_slope = -1;
        }
    }
    else
    {
        if (rand() < (RAND_MAX / 100 * 60))
        {
            // no change
        }
        else if(rand() < (RAND_MAX / 100 * 60))
        {
            sky_slope = 0;
        }
        else
        {
            sky_slope *= -1;
        }
    }
    new_slice.sky_depth += sky_slope;
    if(new_slice.sky_depth > height - 1 )
    {
        // just reverse the slope
        new_slice.sky_depth = height - 2;
    }
    else if(new_slice.sky_depth < (new_slice.ground_height + gap) )
    {
        // just reverse the slope
        new_slice.sky_depth = new_slice.ground_height + gap;
    }


    return new_slice;
}

void generate_terrain(slice_t *terrain, uint16_t x, uint16_t y)
{
    uint16_t counter;
    terrain[0] = generate_terrain_slice((slice_t){0,y-1}, (slice_t){0,y-1}, y);
    terrain[1] = generate_terrain_slice(terrain[0], terrain[0], y);
    for (counter = 2; counter <  x; counter ++)
    {
        terrain[counter] = generate_terrain_slice(terrain[counter - 2], terrain[counter - 1], y);
    }
}
