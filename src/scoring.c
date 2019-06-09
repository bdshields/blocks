/*
 * scoring.c
 *
 *  Created on: 4 Jun. 2019
 *      Author: brandon
 */

#include "scoring.h"
#include <stddef.h>
#include <stdio.h>

score_t scoreboard[NUM_SCORE];

void score_init(const char *title, uint16_t num_players)
{
    uint16_t counter;
    if(num_players > NUM_SCORE)
    {
        num_players = NUM_SCORE;
    }
    for (counter=0; counter<num_players; counter++)
    {
        scoreboard[counter].game = title;
        snprintf(scoreboard[counter].name,NAME_LEN,"Player %d",counter + 1);
        scoreboard[counter].score = 0;
    }
    while(counter < NUM_SCORE)
    {
        scoreboard[counter].game = NULL;
        counter++;
    }
}

/**
 *  Player is zero indexed
 */
void score_set(uint16_t player, int32_t new_score)
{
    if(player < NUM_SCORE)
    {
        if(scoreboard[player].game != NULL)
        {
            scoreboard[player].score = new_score;
        }
    }
}

/**
 *  Player is zero indexed
 */
void score_adjust(uint16_t player, int32_t score_adjustment)
{
    if(player < NUM_SCORE)
    {
        if(scoreboard[player].game != NULL)
        {
            scoreboard[player].score += score_adjustment;
        }
    }
}


void score_save(void)
{
    return;
}

score_t score_player(uint16_t player)
{
    if(player < NUM_SCORE)
    {
         return scoreboard[player];
    }
    else
    {
        return (score_t){.game=NULL};
    }
}


