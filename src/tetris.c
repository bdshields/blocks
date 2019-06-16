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
#include "input.h"
#include "image_util.h"
#include "colours.h"
#include "frame_drv.h"

#include "scoring.h"

const raster_t *block_list[]={
        &tetris_1,
        &tetris_2,
        &tetris_3,
        &tetris_4,
        &tetris_5,
        &tetris_6,
        &tetris_7
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

raster_t *tetris_option(void)
{
    return &tetris_2;
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

void tetris_run(uint16_t x, uint16_t y)
{
    raster_t *game_area;
    raster_t *dropped_blocks;
    raster_t *sprite_block;

    uint16_t    block_index;
    systime     gravity_update;
    uint16_t    gravity;
    pos_t       pos_block;

    tetris_delete_t    remove_lines;
    systime     animate_tmr;
    uint16_t    animate_counter;
    uint16_t    points;
    uint16_t    touching;
    user_input_t    button;
    uint16_t    update_scr=0;

    uint16_t game_state = ts_init_game;

    score_init("Tetris", 1, "Player 1");

    dropped_blocks = fb_allocate(x, y);
    game_area = fb_allocate(x, y);
    sprite_block = fb_allocate(4, 4);
    while(1)
    {
        switch(game_state)
        {
        case ts_init_game:  // Init Game
            clear_raster(dropped_blocks);
            score_set(0, 0);

        case ts_new_block:  // New block
#if 1
            block_index = rand() % 7;
#else
            block_index = 5;
#endif
            clear_raster(sprite_block);
            paste_sprite(sprite_block, block_list[block_index],(pos_t){0,0});
            pos_block = (pos_t){x/2 - 2, 0};

            touching = sprite_touching(dropped_blocks, sprite_block, pos_block);

            gravity = TETRIS_GRAVITY;
            gravity_update = set_alarm(gravity);

            if(touching & down)
            {
                game_state = ts_game_over;
            }
            else
            {
                game_state = ts_none;
            }
            update_scr = 1;
            break;
        case ts_drop_block:
            if(touching & down)
            {
                // Block has touched down
                game_state = ts_attach_block;
            }
            else
            {
                pos_block.y ++;
                game_state = ts_none;
            }
            update_scr = 1;
            gravity_update = set_alarm(gravity);
            break;
        case ts_attach_block:
            paste_sprite(dropped_blocks, sprite_block,pos_block);
            points = tetris_check(dropped_blocks, &remove_lines);
            if(points > 0)
            {
                score_adjust(0, points);
            }
            if(remove_lines.num_deleted)
            {
                gravity_update = cancel_alarm(NULL);
                pos_block = (pos_t){x/2 - 2, -10};
                animate_tmr=set_alarm(100);
                animate_counter = 0;
                update_scr = 1;
                game_state = ts_animate_rows;

            }
            else
            {
                gravity_update = set_alarm(gravity);
                update_scr = 1;
                game_state = ts_new_block;
            }
            break;
        case ts_animate_rows:
            if(alarm_expired(animate_tmr))
            {
                if(animate_counter < dropped_blocks->x_max)
                {
                    uint16_t y;
                    for(y=0; y<remove_lines.num_deleted; y++)
                    {
                        *fb_get_pixel(dropped_blocks,animate_counter, remove_lines.rows[y]) = tetris_animate[remove_lines.num_deleted - 1];
                    }
                    animate_counter++;
                    animate_tmr=set_alarm(50);
                    update_scr = 1;
                }
                else
                {
                    game_state = ts_remove_rows;
                }
            }
            break;
        case ts_remove_rows:
        {
            uint16_t y;
            for(y=0; y<remove_lines.num_deleted; y++)
            {
                memmove(dropped_blocks->image + dropped_blocks->x_max, dropped_blocks->image, remove_lines.rows[y] * dropped_blocks->x_max * sizeof(pixel_t));
            }
            update_scr = 1;
            game_state = ts_new_block;
        }
            break;
        case ts_game_over:
            frame_sleep(1000);
            gravity_update = set_alarm(TETRIS_GRAVITY);
            game_state = ts_init_game;
            break;
        case ts_none:
            button = in_get_bu();
            switch(button.button)
            {
            case bu_left:
                if(!(touching & left))
                {
                    pos_block.x --;
                    update_scr = 1;
                }
                break;
            case bu_right:
                if(!(touching & right))
                {
                    pos_block.x ++;
                    update_scr = 1;
                }
                break;
            case bu_a:
            case bu_up:
                if(sprite_can_rotate(dropped_blocks, sprite_block, pos_block, tr_rot_left))
                {
                    sprite_transform(sprite_block,tr_rot_left);
                    update_scr = 1;
                }
                break;
            case bu_b:
                if(sprite_can_rotate(dropped_blocks, sprite_block, pos_block, tr_rot_right))
                {
                    sprite_transform(sprite_block,tr_rot_right);
                    update_scr = 1;
                }
                break;
            case bu_down:
                game_state = ts_drop_block;
                break;
            case bu_start:
                goto exit;
                break;
            }
            break;
        }
        if(update_scr)
        {
            // Update screen
            clear_raster(game_area);
            paste_sprite(game_area, dropped_blocks,(pos_t){0,0});
            paste_sprite(game_area, sprite_block,pos_block);
            frame_drv_render(game_area);
            update_scr = 0;
        }
        else
        {
            frame_sleep(50);
        }

        if(pos_block.y >= 0)
        {
            touching = sprite_touching(dropped_blocks, sprite_block, pos_block);
        }



        if(alarm_expired(gravity_update))
        {
            game_state = ts_drop_block;
        }

    }

exit:
    score_save();
    fb_destroy(game_area);
    fb_destroy(dropped_blocks);
    fb_destroy(sprite_block);
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
