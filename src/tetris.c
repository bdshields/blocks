/*
 * tetris.c
 *
 *  Created on: 5 May 2019
 *      Author: brandon
 */

#include "tetris.h"
#include "tetris_sprites.h"

#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "pos.h"
#include "button.h"
#include "image_util.h"
#include "colours.h"
#include "frame_drv.h"

#include "scoring.h"
#include "http_session.h"

#include "text.h"

const raster_t *block_list[]={
        &tetris_1,
        &tetris_2,
        &tetris_3,
        &tetris_4,
        &tetris_5,
        &tetris_6,
        &tetris_7
};

const raster_t game_border = {
    .x_max = 1,
    .y_max = 1,
    .image = {PX_YELLO}
};

const raster_t tetris_logo = {
    .x_max = 4,
    .y_max = 4,
    .image = {PX_CLEAR,PX_GREEN,PX_PURPL,PX_CLEAR,
            PX_GREEN,PX_GREEN,PX_PURPL,PX_CLEAR,
            PX_GREEN,PX_YELLO,PX_PURPL,PX_PURPL,
            PX_YELLO,PX_YELLO,PX_YELLO,PX_CLEAR
    }
};

enum tetris_state {
    ts_none,
    ts_init_game,
    ts_new_block,
    ts_drop_block,
    ts_attach_block,
    ts_animate_rows,
    ts_remove_rows,
    ts_game_over,
};

#define TETRIS_GRAVITY 500
#define TETRIS_GRAVITY_MIN 100
#define TETRIS_GAME_WIDTH 10

raster_t *tetris_option(void)
{
    return &tetris_logo;
}

typedef struct tetris_delete_s{
    uint16_t num_deleted;
    uint16_t rows[4];
}tetris_delete_t;

pixel_t tetris_animate[] = {
        PX_CYAN_,
        PX_PURPL,
        PX_YELLO,
        PX_RED__
};


uint16_t tetris_check(raster_t *blocks, tetris_delete_t *details);
pos_t getBlockPos(pos_t pos_block, raster_t *sprite);

#define TETRIS_MAX_PLAYERS 2

void tetris_run(uint16_t x, uint16_t y)
{
    raster_t *game_area;
    raster_t *dropped_blocks[TETRIS_MAX_PLAYERS]={0};
    raster_t *sprite_block[TETRIS_MAX_PLAYERS]={0};

    uint16_t    block_index[TETRIS_MAX_PLAYERS];
    uint16_t    next_block[TETRIS_MAX_PLAYERS];
    systime     gravity_update[TETRIS_MAX_PLAYERS];
    uint16_t    gravity[TETRIS_MAX_PLAYERS];
    pos_t       pos_block[TETRIS_MAX_PLAYERS];
    pos_t       pos_new_block[TETRIS_MAX_PLAYERS];
    pos_t       pos_game[TETRIS_MAX_PLAYERS];

    tetris_delete_t    remove_lines[TETRIS_MAX_PLAYERS];
    systime     animate_tmr[TETRIS_MAX_PLAYERS];
    uint16_t    animate_counter[TETRIS_MAX_PLAYERS];
    uint16_t    points;
    uint16_t    touching[TETRIS_MAX_PLAYERS];
    user_input_t    button;
    uint16_t    update_scr=0;

    uint16_t     num_players=1;
    uint16_t     player_idx=0;

    uint16_t game_state[TETRIS_MAX_PLAYERS] = {ts_init_game, ts_init_game};
    uint16_t next_state[TETRIS_MAX_PLAYERS] = {ts_none, ts_none};

    if(http_session_setPlayers(2) == 2)
    {
        http_session_setTeams(1);
        score_init("Tetris",2, http_session_getTeamName(1), http_session_getTeamName(2));
        num_players = 2;
    }
    else if(http_session_setPlayers(1) == 1)
    {
        http_session_setTeams(1);
        score_init("Tetris",1, http_session_getTeamName(1));
        num_players = 1;
    }
    else
    {
        // Just playing at the terminal
        score_init("Tetris", 1, "Player 1");
        num_players = 1;
    }
    game_area = fb_allocate(x, y);

    for(player_idx = 0; player_idx<num_players; player_idx++)
    {
        dropped_blocks[player_idx] = fb_allocate(TETRIS_GAME_WIDTH, y);
        sprite_block[player_idx] = NULL;
        pos_game[player_idx] = (pos_t){x/2 - ((TETRIS_GAME_WIDTH / 2) * num_players) + (player_idx * (TETRIS_GAME_WIDTH + 1)), 0};

#if 1
        next_block[player_idx] = rand() % 7;
#elif 0
        next_block[player_idx] = (block_index +1) %7;
#else
        next_block[player_idx] = 6;
#endif

    }
    pos_new_block[0] = POS(0,4);
    pos_new_block[1] = POS(x-2,4);

    while(1)
    {
        for (player_idx = 0; player_idx< num_players; player_idx++)
        {
            switch(game_state[player_idx])
            {
            case ts_init_game:  // Init Game
                fb_clear(dropped_blocks[player_idx]);
                score_set(player_idx, 0);
                ;;
            case ts_new_block:  // New block
                block_index[player_idx] = next_block[player_idx];
    #if 1
                next_block[player_idx] = rand() % 7;
    #elif 0
                next_block[player_idx] = (block_index +1) %7;
    #else
                next_block[player_idx] = 6;
    #endif
                sprite_block[player_idx] = fb_copy(block_list[block_index[player_idx]]);
                pos_block[player_idx] = (pos_t){(TETRIS_GAME_WIDTH/2) - 1, 0};

                touching[player_idx] = sprite_touching(dropped_blocks[player_idx], sprite_block[player_idx], getBlockPos(pos_block[player_idx],sprite_block[player_idx]));

                gravity[player_idx] = TETRIS_GRAVITY;
                gravity_update[player_idx] = set_alarm(gravity[player_idx]);

                if(touching[player_idx] & down)
                {
                    game_state[player_idx] = ts_game_over;
                }
                else
                {
                    game_state[player_idx] = ts_none;
                }
                update_scr = 1;
                break;
            case ts_drop_block:
                if(touching[player_idx] & down)
                {
                    // Block has touched down
                    game_state[player_idx] = ts_attach_block;
                }
                else
                {
                    pos_block[player_idx].y ++;
                    game_state[player_idx] = ts_none;
                }
                update_scr = 1;
                gravity_update[player_idx] = set_alarm(gravity[player_idx]);
                break;
            case ts_attach_block:
                paste_sprite(dropped_blocks[player_idx], sprite_block[player_idx],getBlockPos(pos_block[player_idx],sprite_block[player_idx]));
                fb_destroy(sprite_block[player_idx]);
                sprite_block[player_idx] = NULL;
                points = tetris_check(dropped_blocks[player_idx], &remove_lines[player_idx]);
                if(points > 0)
                {
                    score_adjust(player_idx, points);
                }
                if(remove_lines[player_idx].num_deleted)
                {
                    gravity_update[player_idx] = cancel_alarm(NULL);
                    pos_block[player_idx] = (pos_t){-10, -10};
                    animate_tmr[player_idx]=set_alarm(100);
                    animate_counter[player_idx] = 0;
                    update_scr = 1;
                    game_state[player_idx] = ts_animate_rows;

                }
                else
                {
                    gravity_update[player_idx] = set_alarm(gravity);
                    update_scr = 1;
                    game_state[player_idx] = ts_new_block;
                }
                break;
            case ts_animate_rows:
                if(alarm_expired(animate_tmr[player_idx]))
                {
                    if(animate_counter[player_idx] < dropped_blocks[player_idx]->x_max)
                    {
                        uint16_t y;
                        for(y=0; y<remove_lines[player_idx].num_deleted; y++)
                        {
                            *fb_get_pixel(dropped_blocks[player_idx],animate_counter[player_idx], remove_lines[player_idx].rows[y]) = tetris_animate[remove_lines[player_idx].num_deleted - 1];
                        }
                        animate_counter[player_idx]++;
                        animate_tmr[player_idx]=set_alarm(50);
                        update_scr = 1;
                    }
                    else
                    {
                        game_state[player_idx] = ts_remove_rows;
                    }
                }
                break;
            case ts_remove_rows:
            {
                uint16_t y;
                for(y=0; y<remove_lines[player_idx].num_deleted; y++)
                {
                    memmove(dropped_blocks[player_idx]->image + dropped_blocks[player_idx]->x_max, dropped_blocks[player_idx]->image, remove_lines[player_idx].rows[y] * dropped_blocks[player_idx]->x_max * sizeof(pixel_t));
                }
                update_scr = 1;
                game_state[player_idx] = ts_new_block;
            }
                break;
            case ts_game_over:
                if(num_players == 2)
                {
                    uint16_t local_x;
                    uint16_t local_y;
                    for(local_x=0; local_x<TETRIS_GAME_WIDTH; local_x++)
                    {
                        for(local_y=0; local_y<y; local_y++)
                        {
                            *fb_get_pixel(game_area, pos_game[player_idx^1].x+local_x,pos_game[player_idx^1].y+local_y) = PX_CLEAR;
                        }
                    }
                    text_print(game_area,pos_add(pos_game[player_idx^1],POS(0,-2)),PX_PURPL,"W");
                    text_print(game_area,pos_add(pos_game[player_idx^1],POS(3,3)),PX_PURPL,"i");
                    text_print(game_area,pos_add(pos_game[player_idx^1],POS(6,8)),PX_PURPL,"n");
                    frame_drv_render(game_area);
                }
                frame_sleep(3000);
                gravity_update[0] = set_alarm(TETRIS_GRAVITY);
                gravity_update[1] = set_alarm(TETRIS_GRAVITY);
                game_state[0] = ts_init_game;
                game_state[1] = ts_init_game;
                next_state[0] = ts_none;
                next_state[1] = ts_none;
                break;
            case ts_none:
                game_state[player_idx] = next_state[player_idx];
                next_state[player_idx] = ts_none;
                break;
            }

        }

        button = in_get_bu();
        if((button.user >= 1) && (button.user <= num_players))
        {
            player_idx = button.user - 1;
            switch(button.button)
            {
            case bu_left:
                if(!(touching[player_idx] & left))
                {
                    pos_block[player_idx].x --;
                    update_scr = 1;
                }
                break;
            case bu_right:
                if(!(touching[player_idx] & right))
                {
                    pos_block[player_idx].x ++;
                    update_scr = 1;
                }
                break;
            case bu_a:
            case bu_up:
                if(sprite_can_rotate(dropped_blocks[player_idx], sprite_block[player_idx], pos_block[player_idx], tr_rot_left))
                {
                    sprite_transform(sprite_block[player_idx],tr_rot_left);
                    update_scr = 1;
                }
                break;
            case bu_b:
                if(sprite_can_rotate(dropped_blocks[player_idx], sprite_block[player_idx], pos_block[player_idx], tr_rot_right))
                {
                    sprite_transform(sprite_block[player_idx],tr_rot_right);
                    update_scr = 1;
                }
                break;
            case bu_down:
                next_state[player_idx] = ts_drop_block;
                break;
            case bu_start:
                goto exit;
                break;
            }

        }
        else if (button.button == bu_start)
        {
            goto exit;
        }



        if(update_scr)
        {
            int16_t local_y;
            // Update screen
            fb_clear(game_area);
            for(player_idx = 0; player_idx < num_players; player_idx++)
            {
                paste_sprite(game_area, block_list[next_block[player_idx]],pos_new_block[player_idx]);

                paste_sprite(game_area, dropped_blocks[player_idx],pos_game[player_idx]);
                if(sprite_block[player_idx])
                {
                    paste_sprite(game_area, sprite_block[player_idx],pos_add(pos_game[player_idx],getBlockPos(pos_block[player_idx], sprite_block[player_idx])));
                }

                // Add border
                for(local_y=0; local_y<game_area->y_max; local_y++)
                {
                    paste_sprite(game_area, &game_border,(pos_t){pos_game[player_idx].x -1, local_y});
                }
                // Add border
                for(local_y=0; local_y<game_area->y_max; local_y++)
                {
                    paste_sprite(game_area, &game_border,(pos_t){pos_game[player_idx].x + TETRIS_GAME_WIDTH, local_y});
                }
            }
            frame_drv_render(game_area);
            update_scr = 0;
        }
        else
        {
            frame_sleep(50);
        }

        for(player_idx =0; player_idx<num_players; player_idx++)
        {
            if(sprite_block[player_idx] && (pos_block[player_idx].y >= 0))
            {
                touching[player_idx] = sprite_touching(dropped_blocks[player_idx], sprite_block[player_idx], getBlockPos(pos_block[player_idx], sprite_block[player_idx]));
            }

            if(alarm_expired(gravity_update[player_idx]))
            {
                next_state[player_idx] = ts_drop_block;
                cancel_alarm(&gravity_update[player_idx]);
            }
        }
    }

exit:
    score_save();
    fb_destroy(game_area);
    for(player_idx=0; player_idx<TETRIS_MAX_PLAYERS; player_idx++)
    {
        fb_destroy(dropped_blocks[player_idx]);
        fb_destroy(sprite_block[player_idx]);
    }

}

const uint16_t points[]={0, 10,20,50,100};
/**
 * Check blocks that can be removed
 * @param blocks Raster of blocks to check for tetris
 * @param offset Pointer to receive offset where to remove rows
 * @param count  Pointer to receive number of rows to remove
 *
 * @return Points scored
 */
uint16_t tetris_check(raster_t *blocks, tetris_delete_t *deleted)
{
    uint16_t score;
    uint16_t row_counter =0;
    uint16_t x;
    uint16_t y;
    pixel_t *pixel;
    uint16_t run_start = 0;

    score = 0;
    deleted->num_deleted = 0;

    for(y=0; y<blocks->y_max; y++)
    {
        pixel = blocks->image + (y * blocks->x_max);
        for(x=0; x<blocks->x_max; x++)
        {
            if(pixel->flags & R_VISIBLE)
            {
                pixel ++;
            }
            else
            {
                // reset counter
                goto reset_count;
                break;
            }
        }
        row_counter ++;
        continue;
reset_count:
        if(row_counter)
        {
            // counted rows
            score += points[row_counter];
            do{
                deleted->rows[deleted->num_deleted] = run_start;
                deleted->num_deleted++;
                run_start ++;
                row_counter --;
            }while(row_counter > 0);
        }
        row_counter = 0;
        run_start = y+1;
    }
    // counted rows
    if(row_counter)
    {
        // counted rows
        score += points[row_counter];
        do{
            deleted->rows[deleted->num_deleted] = run_start;
            deleted->num_deleted++;
            run_start ++;
            row_counter --;
        }while(row_counter > 0);
    }
    return score;
}

pos_t getBlockPos(pos_t pos_block, raster_t *sprite)
{
    pos_t pos;
    pos = pos_subtract(pos_block, ORI_2_POS(sprite->center));
    return pos;
}
