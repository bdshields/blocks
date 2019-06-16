/*
 * scoring.h
 *
 *  Created on: 4 Jun. 2019
 *      Author: brandon
 */

#ifndef SRC_SCORING_H_
#define SRC_SCORING_H_

#include <stdint.h>
#include <stdarg.h>

#define NAME_LEN    30

#define SCORE_MAXTEAMS 4
#define SCORE_HIST  10   // Number of high scores to retain per game


typedef struct scoreGame_s{
    const char    *game;
    uint8_t        num_teams;
    char          *teamNames;
    int32_t        teamScore[0];
}scoreGame_t;

void score_init(const char *title, uint16_t numTeams, ...);
void score_set(uint16_t player, int32_t new_score);
void score_adjust(uint16_t player, int32_t score_adjustment);
void score_save(void);

/**
 * Retrieve score details of current game
 */
uint16_t score_getNumTeams(void);
int32_t score_getScore(uint16_t team);
char *score_getTeam(uint16_t team);
char *score_getGame(void);


/**
 * Retrieve historic game result
 */
//uint16_t score_getNumTeams(void);




#endif /* SRC_SCORING_H_ */
