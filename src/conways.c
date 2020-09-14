/*
 * conways.c
 *
 *  Created on: 5 Sep 2020
 *      Author: brandon
 *
 *  Conways game of life
 *      https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life
 *
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "colours.h"
#include "frame_drv.h"
#include "frame_buffer.h"
#include "button.h"
#include "utils.h"
#include "image_util.h"

#define CONWAYS_FPS  5



typedef enum _conway_lifeState{
    ls_none,
    ls_death,
    ls_birth,
    ls_mature
} conway_lifeState;

pixel_t cw_states[]={
        PX_BLANK,
        PX_PURPL,
        PX_LITEBLUE,
        PX_WATERBLUE
};


const raster_t conways_logo = {
    .x_max = 4,
    .y_max = 4,
    .image = {PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_RED__,
            PX_CLEAR,PX_CLEAR,PX_RED__,PX_CLEAR,
            PX_PURPL,PX_RED__,PX_PURPL,PX_CLEAR,
            PX_PURPL,PX_PURPL,PX_PURPL,PX_CLEAR
    }
};

raster_t *conways_option(void)
{
    return &conways_logo;
}

uint16_t conway_update(conway_lifeState *dataCurret, conway_lifeState *dataNext, uint16_t x, uint16_t y);
void conway_randomStart(conway_lifeState *data, uint16_t x, uint16_t y);


void conways_run(uint16_t x, uint16_t y)
{
    raster_t        *game_area;
    user_input_t    button;
    systime         animate_tmr;
    conway_lifeState *data[2];
    uint16_t        generation=0;

    uint16_t        index;

    game_area = fb_allocate(x, y);


    data[0] = malloc(sizeof(conway_lifeState) * x * y);
    memset(data[0], 0, sizeof(conway_lifeState) * x * y);
    data[1] = malloc(sizeof(conway_lifeState) * x * y);
    memset(data[1], 0, sizeof(conway_lifeState) * x * y);

    // Seed the data
    conway_randomStart(data[0], x, y);

    animate_tmr = set_alarm(0);
    while(1)
    {
        if(alarm_expired(animate_tmr))
        {
            animate_tmr = set_alarm(1000/CONWAYS_FPS);


            clear_raster(game_area);
            for(index = 0; index<(x*y); index++)
            {
                game_area->image[index] = cw_states[data[generation][index]];
            }
            frame_drv_render(game_area);
            if(conway_update(data[generation],data[(generation+1)&0x01], x, y))
            {
                goto delayed_exit;
            }
            generation ^= 0x01;


        }


        button = in_get_bu();
        switch(button.button)
        {
        case bu_left:
            break;
        case bu_right:
            break;
        case bu_down:
            break;
        case bu_a:
            break;
        case bu_b:
            break;
        case bu_start:
            goto exit;
            break;
        case bu_none:
            frame_sleep(10);
            break;
        }

    }
delayed_exit:
    animate_tmr = set_alarm(5000);
    while( ! alarm_expired(animate_tmr))
    {
        frame_sleep(50);
    }
exit:
    free(data[0]);
    free(data[1]);
    fb_destroy(game_area);
}

uint16_t conway_neighbourCount(conway_lifeState *data, uint16_t x, uint16_t y, uint16_t x_pos, uint16_t y_pos);


/**
 * Rules of growth:
 * 1. Any live cell with fewer than two live neighbours dies, as if by underpopulation.
 * 2. Any live cell with two or three live neighbours lives on to the next generation.
 * 3. Any live cell with more than three live neighbours dies, as if by overpopulation.
 * 4. Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
 *
 * Return 1 if reached end point
 *
 */
#define IDX(_x,_y) (((_y) * x) + _x)
uint16_t conway_update(conway_lifeState *dataCurrent, conway_lifeState *dataNext, uint16_t x, uint16_t y)
{
    uint16_t xIndex;
    uint16_t yIndex;

    conway_lifeState currentState;
    conway_lifeState newState;

    uint16_t neightbours;

    // Track how much has not changed
    uint16_t changed=0;

    for(yIndex=0; yIndex<y; yIndex++)
    {
        for(xIndex=0; xIndex<x; xIndex++)
        {
            neightbours = conway_neighbourCount(dataCurrent, x, y, xIndex, yIndex);
            currentState = dataCurrent[IDX(xIndex, yIndex)];
            // Determine new state
            if(neightbours < 2)
            {
                // death - Under population
                switch(currentState)
                {
                case ls_death:
                case ls_none:
                    newState = ls_none;
                    break;
                case ls_birth:
                case ls_mature:
                    newState = ls_death;
                    break;
                }
            }
            else if(neightbours > 3)
            {
                // death - Over population
                switch(currentState)
                {
                case ls_death:
                case ls_none:
                    newState = ls_none;
                    break;
                case ls_birth:
                case ls_mature:
                    newState = ls_death;
                    break;
                }
            }
            else if(neightbours == 2)
            {
                // sustained
                switch(currentState)
                {
                case ls_death:
                case ls_none:
                    newState = ls_none;
                    break;
                case ls_birth:
                case ls_mature:
                    newState = ls_mature;
                    break;
                }
            }
            else if(neightbours == 3)
            {
                // sustained and growth
                switch(currentState)
                {
                case ls_death:
                case ls_none:
                    newState = ls_birth;
                    break;
                case ls_birth:
                case ls_mature:
                    newState = ls_mature;
                    break;
                }
            }

            if(currentState != newState)
            {
                changed ++;
            }
            dataNext[IDX(xIndex, yIndex)] = newState;
        }
    }
    return changed == 0;
}

/**
 * Scan around specified point to find live neighbours
 *
 *    A   B   C
 *
 *    D   #   E
 *
 *    F   G   H
 *
 */
uint16_t conway_neighbourCount(conway_lifeState *data, uint16_t x, uint16_t y, uint16_t x_pos, uint16_t y_pos)
{
    uint16_t result = 0;
    // A
    if((x_pos>0) && (y_pos>0))
    {
        result += data[IDX(x_pos - 1, y_pos - 1)] >= ls_birth;
    }
    // B
    if((y_pos>0))
    {
        result += data[IDX(x_pos, y_pos - 1)] >= ls_birth;
    }
    // C
    if((x_pos < (x-1)) && (y_pos>0))
    {
        result += data[IDX(x_pos + 1, y_pos - 1)] >= ls_birth;
    }
    // D
    if((x_pos>0))
    {
        result += data[IDX(x_pos - 1, y_pos)] >= ls_birth;
    }
    // E
    if((x_pos < (x-1)))
    {
        result += data[IDX(x_pos + 1, y_pos)] >= ls_birth;
    }
    // F
    if((x_pos>0) && (y_pos < (y-1)))
    {
        result += data[IDX(x_pos - 1, y_pos + 1)] >= ls_birth;
    }
    // G
    if((y_pos < (y-1)))
    {
        result += data[IDX(x_pos, y_pos + 1)] >= ls_birth;
    }
    // H
    if((x_pos < (x-1)) && (y_pos < (y-1)))
    {
        result += data[IDX(x_pos + 1, y_pos + 1)] >= ls_birth;
    }


    return result;
}

void conway_randomStart(conway_lifeState *data, uint16_t x, uint16_t y)
{
    uint16_t  counter;
    uint16_t  numPoints;

    int16_t  xIndex;
    int16_t  yIndex;
    int16_t  newxIndex;
    int16_t  newyIndex;

    uint16_t  stepAttempt;
    // How many points
    numPoints = 60 + (rand() & 0x000F);

    xIndex += (rand()%(x));
    yIndex += (rand()%(y));

    for(counter=0; counter<numPoints; counter++)
    {
        data[IDX(xIndex, yIndex)] = ls_mature;

        // update with a random walk
        stepAttempt = 0;
        do{
            newxIndex = xIndex + (rand() % 5) - 2;
            newyIndex = yIndex + (rand() % 5) - 2;

            if(newxIndex<0)
            {
                newxIndex = 0;
            }
            if(newxIndex >= x)
            {
                newxIndex = x-1;
            }
            if(newyIndex<0)
            {
                newyIndex = 0;
            }
            if(newyIndex >= y)
            {
                newyIndex = y-1;
            }
        } while((data[IDX(newxIndex, newyIndex)] == ls_mature) && (stepAttempt++ < 10));
        xIndex = newxIndex;
        yIndex = newyIndex;
    }

}
