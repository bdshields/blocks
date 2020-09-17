/*
 * invaders.c
 *
 *  Created on: 15 May 2019
 *      Author: brandon
 */

#include "invaders.h"
#include "invaders_sprites.h"
#include <stdlib.h>
#include "utils.h"
#include "pos.h"
#include "button.h"
#include "image_util.h"
#include "frame_drv.h"
#include "colours.h"
#include "scoring.h"
#include "http_session.h"

#define ALIEN_HEADROOM 0  // Space above aliens
#define ALIEN_FOOTER   5  // Space between aliens and cannon
#define ALIEN_SPACE 1     // Space inbetween
#define ALIEN_SWING 12     // Lateral movement
#define ALIEN_MAX_ROWS 5

#define CANNON_HEADROOM 1 // Space below cannon

#define BOMB_SPEED 50
#define ALIEN_SPEED 1000
#define ALIEN_MAX_SPEED 200


enum invader_state {
    is_none,
    is_init_game,
    is_playing,
    is_shoot,
    is_hit,
    is_game_over,
    is_game_win
};



raster_t *invaders_option(void)
{
    return &invader_logo;
}

uint16_t generate_aliens(raster_t * aliens);

void invaders_run(uint16_t x, uint16_t y)
{
    raster_t *game_area;
    raster_t *aliens;

    pos_t   pos_cannon;
    pos_t   pos_alien;
    pos_t   pos_bomb = {-1, -1};

    systime     bomb_move;
    systime     alien_move;
    systime     win_animate;
    uint16_t    alien_touch;
    uint16_t    cannon_touch;
    dir_t       alien_dir=right;
    uint16_t    bomb_hit;
    uint16_t    update_scr;
    user_input_t    button;
    uint16_t    dead_counter;
    uint16_t    total_aliens;
    enum invader_state game_state = is_init_game;

    if(http_session_setPlayers(1) == 1)
    {
        http_session_setTeams(1);
        score_init("Invaders", 1,http_session_getTeamName(1));
    }
    else
    {
        score_init("Invaders", 1,"Player 1");
    }
    game_area = fb_allocate(x, y);
    aliens = fb_allocate(x-ALIEN_SWING, y - invader_cannon.y_max - CANNON_HEADROOM - ALIEN_HEADROOM - ALIEN_FOOTER);
    while(1)
    {
        switch(game_state)
        {
        case is_init_game:
            update_scr = 1;
            pos_bomb = (pos_t){-1, -1};
            pos_cannon = (pos_t){game_area->x_max/2, game_area->y_max-1-CANNON_HEADROOM};
            pos_alien = (pos_t){ALIEN_SWING/2, ALIEN_HEADROOM};
            alien_touch = 0;
            total_aliens = generate_aliens(aliens);
            dead_counter = 0;
            alien_move = set_alarm(ALIEN_SPEED);
            game_state = is_playing;
            break;
        case is_shoot:
            pos_bomb = pos_cannon;
            pos_bomb.y -= 1;
            bomb_move = set_alarm(BOMB_SPEED);
            update_scr = 1;
            game_state = is_playing;
            break;
        case is_hit:
            if(pos_bomb.y > 0)
            {
                fb_get_pixel(aliens, pos_bomb.x-pos_alien.x, pos_bomb.y-pos_alien.y-1)[0]= PX_CLEAR;
                dead_counter ++;
                score_adjust(0, 100);
            }
            pos_bomb.y = -1;
            game_state = is_playing;
            update_scr = 1;
            break;
        case is_playing:
            // Check game over
            if(dead_counter >= total_aliens)
            {
                cancel_alarm(&bomb_move);
                cancel_alarm(&alien_move);
                game_state = is_game_win;
                win_animate = set_alarm(3000);
            }
            else if(cannon_touch & (left|right))
            {
                //something touching cannon, make sure it isn't a wall
                if((pos_cannon.x > 0 ) && (pos_cannon.x + invader_cannon.x_max) < (game_area->x_max))
                {
                    game_state = is_game_over;
                }
            }
            else if(cannon_touch & (up))
            {
                game_state = is_game_over;
            }
            else if(alien_touch & down)
            {
                game_state = is_game_over;
            }

            // check timers
            if(alarm_expired(bomb_move))
            {
                if(pos_bomb.y != -1)
                {
                    pos_bomb.y -= 1;
                    update_scr = 1;
                }
                bomb_move = set_alarm(BOMB_SPEED);
            }
            if(alarm_expired(alien_move))
            {
                int32_t delay;
                delay = ALIEN_SPEED - (dead_counter * 25);
                if(delay < ALIEN_MAX_SPEED)
                {
                    delay = ALIEN_MAX_SPEED;
                }

                if(alien_dir == left)
                {
                    if(alien_touch & left)
                    {
                        alien_dir = right;
                        pos_alien.y ++;
                        delay >>=1;
                    }
                    else
                    {
                        pos_alien.x --;
                    }
                }
                else if(alien_dir == right)
                {
                    if(alien_touch & right)
                    {
                        alien_dir = left;
                        pos_alien.y ++;
                        delay >>=1;
                    }
                    else
                    {
                        pos_alien.x ++;
                    }
                }
                update_scr = 1;
                alien_move = set_alarm(delay);
            }
            break;
        case is_game_win:
            if(alarm_expired(win_animate))
            {
                frame_sleep(500);
                cancel_alarm(&win_animate);
                game_state = is_init_game;
            }
            else
            {
                pixel_t *pixel;
                score_adjust(0, 100);
                pixel = game_area->image + rand() % (game_area->x_max * game_area->y_max);

                pixel->red=rand() & 0xFF;
                pixel->green=rand() & 0xFF;
                pixel->blue=rand() & 0xFF;
                pixel->flags=R_VISIBLE;
                frame_drv_render(game_area);
            }
            break;
        case is_game_over:
            frame_sleep(1000);
            score_set(0, 0);
            cancel_alarm(&alien_move);
            game_state = is_init_game;
            break;
        case is_none:
            if(update_scr == 0)
            {
                frame_sleep(50);
            }
            break;
        }

        if((game_state != is_game_win)&&(game_state !=is_game_over ))
        {
            button = in_get_bu();
            if(button.user == 1)
            {
                switch(button.button)
                {
                case bu_left:
                    if(pos_cannon.x > 0)
                    {
                        pos_cannon.x--;
                        update_scr = 1;
                    }
                    break;
                case bu_right:
                    if((pos_cannon.x + invader_cannon.x_max) < (game_area->x_max))
                    {
                        pos_cannon.x++;
                        update_scr = 1;
                    }
                    break;
                case bu_a:
                    if(pos_bomb.y == -1)
                    {
                        game_state = is_shoot;
                    }
                    break;
                case bu_start:
                    goto exit;
                    break;
                }
            }

        }

        if(update_scr)
        {
            fb_clear(game_area);
            alien_touch = sprite_touching(game_area, aliens, pos_alien);
            paste_sprite(game_area, aliens,pos_alien);
            cannon_touch = sprite_touching(game_area, &invader_cannon, pos_cannon);
            paste_sprite(game_area, &invader_cannon,pos_cannon);
            if(pos_bomb.y != -1)
            {
                bomb_hit = sprite_touching(game_area, &invader_bomb, pos_bomb);
                paste_sprite(game_area, &invader_bomb,pos_bomb);
                if(bomb_hit & (up))
                {
                    game_state = is_hit;
                    cancel_alarm(&bomb_move);
                }
            }
            frame_drv_render(game_area);
            update_scr = 0;
        }
        else
        {
            frame_sleep(50);

        }




    }
exit:
    score_save();
    fb_destroy(game_area);
    fb_destroy(aliens);
//    fb_destroy(sprite_block);
}

uint16_t generate_aliens(raster_t * aliens)
{
    pos_t pos;
    uint16_t row_count;
    uint16_t alien_counter=0;
    fb_clear(aliens);
    pos.y = 0;
    row_count = 0;

    while(((pos.y + invader_alien.y_max) < aliens->y_max) && (row_count < ALIEN_MAX_ROWS))
    {
        pos.x = 0;
        while(pos.x+ invader_alien.x_max < aliens->x_max)
        {
            paste_sprite(aliens, &invader_alien,pos);
            pos.x += invader_alien.x_max + ALIEN_SPACE;
            alien_counter ++;
        }
        pos.y += invader_alien.y_max + ALIEN_SPACE;
        row_count ++;
    }
    return alien_counter;
}
