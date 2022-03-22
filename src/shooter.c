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
    ss_shooting,
    ss_missile_exploding,
    ss_target_exploding,
    ss_move_up,
    ss_move_down,
    ss_crash,
    ss_game_win
};

enum shooter_event {
    se_start,
    se_move_up,
    se_move_down,
    se_shoot,
    se_missile_move,
    se_missile_hit,
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

/**   Positions measured from the roof
 *    -------------------   0                     0
 *         |
 *         `----------------Roof Drop             2
 *         `----------------Roof Turret           3
 *         :
 *         :
 *         :----------------Gap                   6
 *         ,----------------Floor Turret          7
 *         ,----------------Floor Start           8
 *         |
 *         |
 *   -------------------    Y                     11
 */

typedef struct {
    int16_t roofDrop;
    int16_t  roofTurret;
    int16_t gap;
    int16_t floorTurret;
    int16_t floorStart;
}slice_t;
slice_t generate_terrain_slice(slice_t slice1, slice_t slice2, int height);

void generate_terrain(slice_t *terrain, uint16_t x, uint16_t y);

#define SHIP_XPOS 2
#define SCROLL_DELAY 100

#define SHOOT_DELAY (SCROLL_DELAY/4)


void shooter_run(uint16_t x, uint16_t y)
{
    raster_t *game_area;
    raster_t *terrain_area;
    user_input_t button;
    int       update_scr=0;
    systime     timeout;
    systime     scroll_timeout;
    systime     shoot_timeout;
    int16_t     ship_pos = y/2;
    pos_t       missile_pos;

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
    cancel_alarm(&shoot_timeout);
    while(1)
    {
        // Process events
        enum shooter_event tempEvt = game_event;
        game_event = se_none;
        switch(tempEvt)
        {
        case se_start:
            ship_pos = y/2;
            missile_pos = POS(-1,-1);
            first_slice = 0;
            generate_terrain(shooter_terrain, x, y);
            update_scr = 1;
            game_state = ss_start_delay;
            timeout = set_alarm(2000);
            break;
        case se_scroll:
            shooter_terrain[first_slice % x] = generate_terrain_slice(shooter_terrain[(first_slice + x - 2)%x],shooter_terrain[(first_slice + x - 1)%x], y-1);
            first_slice ++;
            first_slice %= x;
            scroll_timeout = set_alarm(SCROLL_DELAY);
            if((game_state == ss_missile_exploding) || (game_state == ss_target_exploding))
            {
                // Scroll the explosion with the terrain
                missile_pos.x --;
            }
            update_scr = 1;
            break;
        case se_move_up:
            if((game_state == ss_run) || (game_state == ss_shooting) || (game_state == ss_missile_exploding)  || (game_state == ss_target_exploding))
            {
                ship_pos --;
                update_scr = 1;
            }
            break;
        case se_move_down:
            if((game_state == ss_run) || (game_state == ss_shooting) || (game_state == ss_missile_exploding)  || (game_state == ss_target_exploding))
            {
                ship_pos ++;
                update_scr = 1;
            }
            break;
        case se_shoot:
            if (game_state == ss_run)
            {
                missile_pos = POS(SHIP_XPOS+1, ship_pos);
                update_scr = 1;
                game_state = ss_shooting;
                shoot_timeout = set_alarm(SHOOT_DELAY);
            }
            break;
        case se_missile_move:
            if(game_state == ss_shooting)
            {
                missile_pos.x ++;
                if(missile_pos.x < x)
                {
                    shoot_timeout = set_alarm(SHOOT_DELAY);
                }
                else
                {
                    missile_pos.x = -1;
                    game_state = ss_run;
                    cancel_alarm(&shoot_timeout);
                }
                update_scr = 1;
            }
            else if(game_state == ss_missile_exploding || game_state == ss_target_exploding)
            {
                missile_pos.x = -1;
                game_state = ss_run;
                update_scr = 1;
                cancel_alarm(&shoot_timeout);
            }
            break;
        case se_missile_hit:
            if(game_state == ss_shooting)
            {
                if(((missile_pos.y > shooter_terrain[(first_slice + missile_pos.x) % x].roofDrop) &&
                        (missile_pos.y <= shooter_terrain[(first_slice + missile_pos.x) % x].roofTurret)))
                {
                    shooter_terrain[(first_slice + missile_pos.x) % x].roofTurret = shooter_terrain[(first_slice + missile_pos.x) % x].roofDrop;
                    game_state = ss_target_exploding;
                }
                else if(((missile_pos.y > shooter_terrain[(first_slice + missile_pos.x) % x].gap) &&
                        (missile_pos.y <= shooter_terrain[(first_slice + missile_pos.x) % x].floorTurret)))
                {
                    shooter_terrain[(first_slice + missile_pos.x) % x].gap = shooter_terrain[(first_slice + missile_pos.x) % x].floorTurret;
                    game_state = ss_target_exploding;
                }
                else
                {
                    game_state = ss_missile_exploding;
                }
                update_scr = 1;
                shoot_timeout = set_alarm(SHOOT_DELAY);
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
                game_event = se_shoot;
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
        else if(alarm_expired(shoot_timeout))
        {
            game_event = se_missile_move;
            cancel_alarm(&shoot_timeout);
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
                    if (y_cntr <= shooter_terrain[(first_slice + x_cntr) % x].roofDrop)
                    {
                        *fb_get_pixel(terrain_area, x_cntr, y_cntr) = PX_LITEBLUE;
                    }
                    else if (y_cntr <= shooter_terrain[(first_slice + x_cntr) % x].roofTurret)
                    {
                        *fb_get_pixel(terrain_area, x_cntr, y_cntr) = PX_CYAN_;
                    }
                    else if (y_cntr <= shooter_terrain[(first_slice + x_cntr) % x].gap)
                    {
                        // Do nothing
                    }
                    else if (y_cntr <= shooter_terrain[(first_slice + x_cntr) % x].floorTurret)
                    {
                        *fb_get_pixel(terrain_area, x_cntr, y_cntr) = PX_PURPL;
                    }
                    else if (y_cntr >= shooter_terrain[(first_slice + x_cntr) % x].floorStart)
                    {
                        *fb_get_pixel(terrain_area, x_cntr, y_cntr) = PX_GREEN;
                    }
                }
            }
            // SHIP
            *fb_get_pixel(terrain_area, SHIP_XPOS, ship_pos) = PX_RED__;
            // MISSLE
            if(game_state == ss_shooting)
            {
                *fb_get_pixel(terrain_area, missile_pos.x, missile_pos.y) = PX_YELLO;
            }
            else if(game_state == ss_missile_exploding && !pos_out_raster(terrain_area, missile_pos))
            {
                // Animate Explosion
                *fb_get_pixel(terrain_area, missile_pos.x-1, missile_pos.y) = PX_ORANG;
                *fb_get_pixel(terrain_area, missile_pos.x+1, missile_pos.y) = PX_ORANG;
                *fb_get_pixel(terrain_area, missile_pos.x, missile_pos.y-1) = PX_ORANG;
                *fb_get_pixel(terrain_area, missile_pos.x, missile_pos.y+1) = PX_ORANG;
            }
            else if(game_state == ss_target_exploding)
            {
                // Animate Explosion
                *fb_get_pixel(terrain_area, missile_pos.x-1, missile_pos.y) = PX_ORANG;
                *fb_get_pixel(terrain_area, missile_pos.x+1, missile_pos.y) = PX_ORANG;
                *fb_get_pixel(terrain_area, missile_pos.x, missile_pos.y-1) = PX_ORANG;
                *fb_get_pixel(terrain_area, missile_pos.x, missile_pos.y+1) = PX_ORANG;
            }
            // Check for collission
            if((ship_pos <= shooter_terrain[(first_slice + SHIP_XPOS) % x].roofTurret) ||
                    (ship_pos > shooter_terrain[(first_slice + SHIP_XPOS) % x].gap))
            {
                game_event = se_crash;
            }
            else if((game_state == ss_shooting) &&
                    ((missile_pos.y <= shooter_terrain[(first_slice + missile_pos.x) % x].roofTurret) ||
                            (missile_pos.y > shooter_terrain[(first_slice + missile_pos.x) % x].gap)))
            {
                game_event = se_missile_hit;
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

slice_t generate_terrain_slice(slice_t slice1, slice_t slice2, int maxDrop)
{
    slice_t new_slice;
    int16_t ground_slope = 0;  // positive value is down hill
    int16_t sky_slope = 0;     // positive value is downwards
    int16_t gap = GAP;
    int16_t floorPoint = 0;
    int16_t roofPoint = 0;

    ground_slope = slice2.floorStart - slice1.floorStart;
    sky_slope = slice2.roofDrop - slice1.roofDrop;
    new_slice = slice2;
    new_slice.floorTurret = 0;
    new_slice.roofTurret = 0;
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
    if (slice2.floorTurret != slice2.gap)
    {
        // just built a turret on a peak, so must slope downwards
        ground_slope = 1;
    }
    else if (ground_slope == 0)
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
            if (ground_slope == -1)
            {
                floorPoint = 1;
            }
        }
        else if(rand() < (RAND_MAX / 100 * 60))
        {
            // go flat
            ground_slope = 0;
        }
        else
        {
            ground_slope *= -1;
        }
    }

    // Widen minimum gap when slopey
    gap += (ground_slope * ground_slope);

    new_slice.floorStart += ground_slope;
    if(new_slice.floorStart > maxDrop )
    {
        // just reverse the slope
        new_slice.floorStart = maxDrop;
    }
    else if(new_slice.floorStart < new_slice.roofDrop + gap )
    {
        // just reverse the slope
        new_slice.floorStart = new_slice.roofDrop + gap;
    }
    if(floorPoint && (rand() < RAND_MAX / 100 * 30))
    {
        new_slice.floorTurret = new_slice.floorStart - 1;
        new_slice.gap = new_slice.floorStart - 2;
    }
    else
    {
        new_slice.gap = new_slice.floorStart - 1;
        new_slice.floorTurret = new_slice.gap;
    }



    if (slice2.roofTurret != slice2.roofDrop)
    {
        // just built a turret on a peak, so must slope upwards
        sky_slope = -1;
    }
    else if (sky_slope == 0)
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
            if(sky_slope == 1)
            {
                roofPoint = 1;
            }
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

    new_slice.roofDrop += sky_slope;
    if(new_slice.roofDrop < 0 )
    {
        // just reverse the slope
        new_slice.roofDrop = 0;
    }
    else if(new_slice.roofDrop > (new_slice.floorStart - gap) )
    {
        // just reverse the slope
        new_slice.roofDrop = new_slice.floorStart - gap;
    }

    if(roofPoint && (rand() < RAND_MAX / 100 * 30))
    {
        new_slice.roofTurret = new_slice.roofDrop + 1;
    }
    else
    {
        new_slice.roofTurret = new_slice.roofDrop;
    }


    return new_slice;
}

void generate_terrain(slice_t *terrain, uint16_t x, uint16_t y)
{
    uint16_t counter;
    terrain[0] = generate_terrain_slice((slice_t){.roofDrop=0, .floorStart=y-1}, (slice_t){.roofDrop=0, .floorStart=y-1}, y-1);
    terrain[1] = generate_terrain_slice(terrain[0], terrain[0], y-1);
    for (counter = 2; counter <  x; counter ++)
    {
        terrain[counter] = generate_terrain_slice(terrain[counter - 2], terrain[counter - 1], y-1);
    }
}
