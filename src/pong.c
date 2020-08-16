/*
 * pong.c
 *
 *  Created on: 23 May 2019
 *      Author: brandon
 */

#include <math.h>
#include <stdlib.h>
#include "pong.h"
#include "pong_sprites.h"
#include "utils.h"
#include "pos.h"
#include "input.h"
#include "image_util.h"
#include "colours.h"
#include "frame_drv.h"
#include "frame_buffer.h"
#include "scoring.h"
#include "http_session.h"

typedef enum pong_state_e{
    ps_none,
    ps_init_game,
    ps_end_match,
    ps_start_wait,
    ps_start_match
}pong_state_t;

#define BALL_RATE 1500   // Milliseconds to traverse field

raster_t *pong_option(void)
{
    return &pong_logo;
}

void pong_calc_increments(float angle, float *inc_x, float *inc_y);
float random_angle(float min, float max);


void pong_run(uint16_t x, uint16_t y)
{
    raster_t *game_area;


    float           ball_x;
    float           ball_y;
    float           ball_inc_x;
    float           ball_inc_y;

    /**
     * Angle is in radians relative to 12 O'clock.
     * clockwise is positive angle
     * Counter-Clockwise is negative angle
     */
    float           ball_angle;
    systime         ball_tmr;

    pos_t           pos_player[2];
    uint16_t        who_won;
    uint16_t        num_players;

    uint16_t        update_scr;
    pong_state_t    state=ps_init_game;
    user_input_t    button;


    if(http_session_setPlayers(2) == 2)
    {
        http_session_setTeams(1);
        score_init("Pong",2, http_session_getTeamName(1), http_session_getTeamName(2));
        num_players = 2;
    }
    else if(http_session_setPlayers(1) == 1)
    {
        http_session_setTeams(1);
        score_init("Pong",1, http_session_getTeamName(1));
        num_players = 1;
    }
    else
    {
        score_init("Pong",1, "Player 1");
        num_players = 1;
    }

    game_area = fb_allocate(x, y);

    while(1)
    {
        switch(state)
        {
        case ps_init_game:
            pos_player[1] = (pos_t){x/2,2};     // Top player
            pos_player[0] = (pos_t){x/2,y-3};   // Bottom player
            who_won = 0;
            ball_x = x/2;
            ball_y = y-4;
            cancel_alarm(&ball_tmr);
            score_set(0,0);
            score_set(1,0);
            update_scr = 1;
            state = ps_start_wait;
            break;
        case ps_end_match:
            cancel_alarm(&ball_tmr);
            // who won
            if(roundf(ball_y) <= 0)
            {
                // Top player missed ball, bottom player won
                who_won = 0;
                score_adjust(0,5);
                ball_x = x/2;
                ball_y = y-4;
            }
            else
            {
                // Bottom player missed ball
                if(num_players == 2)
                {
                    who_won = 1;
                    score_adjust(1,5);
                    ball_x = x/2;
                    ball_y = 3;
                }
                else
                {
                    // Only one player
                    who_won = 0;
//                    score_adjust(1,5);
                    ball_x = x/2;
                    ball_y = y-4;
                }
            }
            // reset player positions
            pos_player[1] = (pos_t){x/2,2};     // Top player
            pos_player[0] = (pos_t){x/2,y-3};   // Bottom player

            update_scr = 1;
            state = ps_start_wait;
            break;
        case ps_start_match:
            if(who_won)
            {
                ball_angle = random_angle(M_PI * 0.9, M_PI * 1.1);
            }
            else
            {
                ball_angle = random_angle(-(M_PI * 0.1), M_PI * 0.1);
            }
            ball_tmr = set_alarm(BALL_RATE / y);
            pong_calc_increments(ball_angle, &ball_inc_x, &ball_inc_y);
            update_scr = 1;
            state = ps_none;
            break;
        case ps_none:
            break;
        }
        if(alarm_expired(ball_tmr))
        {
            uint16_t update_inc = 0;
            ball_tmr = set_alarm(BALL_RATE / y);
            ball_x += ball_inc_x;
            ball_y += ball_inc_y;

            // Check walls
            if(! (roundf(ball_x) < (x-1)))
            {
                // Contact right side
                if(ball_inc_x > 0)
                {
                    // If heading into wall, bounce off it
                    ball_angle = -ball_angle + random_angle(-0.05, 0.05);
                    update_inc = 1;
                }
            }
            if(! (roundf(ball_x) > 0))
            {
                if(ball_inc_x < 0)
                {
                    // If heading into wall, bounce off it
                    ball_angle = -ball_angle + random_angle(-0.05, 0.05);
                    update_inc = 1;
                }
            }

            // Ball goes out of play
            if(! (roundf(ball_y) < (y-1)))
            {
                state = ps_end_match;
            }
            if(! (roundf(ball_y) > 0))
            {
                if(num_players == 2)
                {
                        state = ps_end_match;
                }
                else
                {
                    // one player, so bounce from the top
                    ball_angle = M_PI-ball_angle+random_angle(-0.05, 0.05);
                    update_inc = 1;
                }
            }

            // Check for paddle
            // Top Player
            if(num_players == 2)
            {
                if(roundf(ball_y) == (pos_player[1].y + 1))
                {
                    if((roundf(ball_x) >= pos_player[1].x) && (roundf(ball_x) < (pos_player[1].x + pong_paddle.x_max)))
                    {
                        ball_angle = M_PI-ball_angle+random_angle(-0.05, 0.05);
                        update_inc = 1;
                    }
                }
            }
            // Bottom player
            if(roundf(ball_y) == (pos_player[0].y - 1))
            {
                if((roundf(ball_x) >= pos_player[0].x) && (roundf(ball_x) < (pos_player[0].x + pong_paddle.x_max)))
                {
                    ball_angle = M_PI-ball_angle+random_angle(-0.05, 0.05);
                    update_inc = 1;
                }
            }
            if(update_inc)
            {
                // Update ball movement with new angle
                pong_calc_increments(ball_angle, &ball_inc_x, &ball_inc_y);
                update_inc = 0;
            }
            update_scr = 1;
        }
        else
        {
            frame_sleep(10);
        }
        if(update_scr)
        {
            clear_raster(game_area);
            paste_sprite(game_area, &pong_paddle,pos_player[0]);
            if(num_players == 2)
            {
                paste_sprite(game_area, &pong_paddle,pos_player[1]);
            }

            fb_get_pixel(game_area, roundf(ball_x), roundf(ball_y))[0] = PX_GREEN;


            frame_drv_render(game_area);
            update_scr = 0;
        }

        button = in_get_bu();
        if(button.user > 0)
        {
            int16_t player;
            player = button.user-1;
            switch(button.button)
            {
            case bu_left:
                if(pos_player[player].x > 0)
                {
                    pos_player[player].x--;
                    if((state == ps_start_wait)&&(who_won == player))
                    {
                        // move the ball also if attached to player
                        ball_x--;
                    }
                    update_scr = 1;
                }
                break;
            case bu_right:
                if((pos_player[player].x + pong_paddle.x_max) < (game_area->x_max))
                {
                    pos_player[player].x++;
                    if((state == ps_start_wait)&&(who_won == player))
                    {
                        // move the ball also if attached to player
                        ball_x++;
                    }
                    update_scr = 1;
                }
                break;
            case bu_a:
                if((state == ps_start_wait)&&(who_won == player))
                {
                    state = ps_start_match;
                }
                break;
            case bu_start:
                goto exit;
                break;
            }
        }

    }



exit:
    score_save();
    fb_destroy(game_area);
}

/**
 * Angle is in radians relative to 12 O'clock.
 * clockwise is positive angle
 * Counter-Clockwise is negative angle
 */
void pong_calc_increments(float angle, float *inc_x, float *inc_y)
{
    *inc_x = sinf(angle);
    *inc_y = -cosf(angle);
}

/**
 * min,max is angle in radians
 */
float random_angle(float min, float max)
{
    float angle;

    angle = (((float)rand()/(float)RAND_MAX) * (max - min)) + min;

    return angle;
}
