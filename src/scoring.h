/*
 * scoring.h
 *
 *  Created on: 4 Jun. 2019
 *      Author: brandon
 */

#ifndef SRC_SCORING_H_
#define SRC_SCORING_H_

#include <stdint.h>

#define NAME_LEN    30

#define NUM_SCORE   4

typedef struct score_s{
    const char    *game;
    char     name[NAME_LEN];
    int32_t  score;
}score_t;

void score_init(const char *title, uint16_t num_players);
void score_set(uint16_t player, int32_t new_score);
void score_adjust(uint16_t player, int32_t score_adjustment);
void score_save(void);

/**
 * Retrieve score for a player
 */
score_t score_player(uint16_t player);



#endif /* SRC_SCORING_H_ */
