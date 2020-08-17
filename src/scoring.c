/*
 * scoring.c
 *
 *  Created on: 4 Jun. 2019
 *      Author: brandon
 */

#include "scoring.h"
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "config_file.h"

scoreGame_t *scoreCurrent=NULL;


scoreGame_t *score_Vallocate(const char *title, uint16_t numTeams, va_list va_names);
scoreGame_t *score_allocate(const char *title, uint16_t numTeams, ...);
char *score_getTeam(scoreGame_t* score, uint16_t team);


void score_init(const char *title, uint16_t numTeams, ...)
{
    va_list  va_getNames;
    va_start (va_getNames, numTeams);

    scoreCurrent = score_Vallocate(title, numTeams, va_getNames);

}

scoreGame_t *score_allocate(const char *title, uint16_t numTeams, ...)
{
    va_list  va_getNames;
    va_start (va_getNames, numTeams);

    return score_Vallocate(title, numTeams, va_getNames);

}

scoreGame_t *score_Vallocate(const char *title, uint16_t numTeams, va_list va_names)
{
    uint16_t counter;
    char     *teamName;
    uint32_t nameLengths = 0;
    va_list  va_getLength;
    scoreGame_t *newScore;

    // duplicate the VA
    va_copy(va_getLength, va_names);

    // Calcualte length of names
    for(counter=0; counter<numTeams; counter++)
    {
        teamName = va_arg(va_getLength,char*);
        nameLengths += strlen(teamName) + 1;
    }
    va_end(va_getLength);

    newScore = (scoreGame_t *)malloc(sizeof(scoreGame_t) + (sizeof(int32_t) * numTeams) + nameLengths);

    // populate the structure
    newScore->game = title;
    newScore->num_teams = numTeams;
    newScore->teamNames = (char*)(newScore->teamScore + numTeams);

    nameLengths = 0;
    for (counter=0; counter<numTeams; counter++)
    {
        // init score
        newScore->teamScore[counter] = 0;
        // copy in team name
        teamName = va_arg(va_names,char*);
        strcpy(newScore->teamNames + nameLengths,teamName);
        nameLengths += strlen(teamName) + 1;
    }
    va_end(va_getLength);
    va_end(va_names);
    return newScore;
}


/**
 *  Player is zero indexed
 */
void score_set(uint16_t player, int32_t new_score)
{
    if(scoreCurrent)
    {
        if(player < scoreCurrent->num_teams)
        {
            scoreCurrent->teamScore[player] = new_score;
        }
    }
}

/**
 *  Player is zero indexed
 */
void score_adjust(uint16_t player, int32_t score_adjustment)
{
    if(scoreCurrent)
    {
        if(player < scoreCurrent->num_teams)
        {
            scoreCurrent->teamScore[player] += score_adjustment;
        }
    }
}


void score_save(void)
{
    scoreGame_t *topScores[SCORE_HIST + 1];   // Enough space for top 10 plus current match.
    char teamName[SCORE_MAXTEAMS][500];

    int32_t    index=0;
    int32_t    numMatches=0;
    int32_t    numTeams=0;

    int32_t    indexMatch;
    int32_t    indexStore;
    int32_t    indexTeam;

    int32_t    scoreMatch=0;
    int32_t    insertMatch=0;
    int32_t    insertDone=0;

    if(scoreCurrent)
    {
        // Get the winning score
        for(indexTeam=0; indexTeam<scoreCurrent->num_teams; indexTeam++)
        {
            if(scoreCurrent->teamScore[indexTeam] > scoreMatch)
            {
                // yes, new high score
                scoreMatch = scoreCurrent->teamScore[indexTeam];
            }
        }

        memset(&topScores,0,sizeof(topScores));

        // Does game have entry in storage
        config_get_int(_k("%s_index",scoreCurrent->game),&index);
        if(index == 0)
        {
            // add the game
            index = 0;
            config_get_int("scoreGameNum",&index);
            index ++;
            // Store new number of games
            config_put_int("scoreGameNum", index);
            // Store new game title
            config_put_string(_k("scoreGameTitle%d",index),scoreCurrent->game);
            // Store game cross reference
            config_put_int(_k("%s_index",scoreCurrent->game), index);
        }

        indexStore = 0;

        if(scoreCurrent->num_teams > 0)
        {

            // Get number of saved scores
            config_get_int(_k("%s_scoreLen",scoreCurrent->game),&numMatches);
            // load all the scores in
            if(numMatches>0)
            {
                if(numMatches > SCORE_HIST)
                {
                    numMatches = SCORE_HIST;
                }
                for(indexMatch=0; indexMatch<numMatches; indexMatch++)
                {
                    // get number of teams for match
                    numTeams = 0;
                    config_get_int(_k("%s_score%dteamNum",scoreCurrent->game,indexMatch+1),&numTeams);
                    if(numTeams == 0)
                    {
                        continue;
                    }
                    if(numTeams > SCORE_MAXTEAMS)
                    {
                        numTeams = SCORE_MAXTEAMS;
                    }
                    // Read team names
                    for(indexTeam=0; indexTeam<numTeams; indexTeam++)
                    {
                        teamName[indexTeam][0]='\0';
                        config_get_string(_k("%s_score%dteam%d",scoreCurrent->game,indexMatch+1,indexTeam+1), teamName[indexTeam]);
                    }
                    // create score object
                    topScores[indexStore] = score_allocate(scoreCurrent->game, numTeams, teamName[0],teamName[1],teamName[2],teamName[3]);
                    // set the scores
                    insertMatch = 1;
                    for(indexTeam=0; indexTeam<numTeams; indexTeam++)
                    {
                        teamName[indexTeam][0]='\0';
                        config_get_int(_k("%s_score%dresult%d",scoreCurrent->game,indexMatch+1,indexTeam+1), &topScores[indexStore]->teamScore[indexTeam]);
                        // Check score against current match
                        if(scoreMatch < topScores[indexStore]->teamScore[indexTeam])
                        {
                            insertMatch = 0;
                        }
                    }
                    // do we need to insert match
                    if(insertMatch && (! insertDone))
                    {
                        // Insert current match into the history, and bump the rest down
                        scoreGame_t *temp;
                        temp = topScores[indexStore];
                        topScores[indexStore] = scoreCurrent;
                        indexStore ++;
                        topScores[indexStore] = temp;
                        insertDone =1;
                    }
                    indexStore ++;
                }
            }
            // tack new score on the end if space
            if((indexStore < SCORE_HIST) && (! insertDone))
            {
                topScores[indexStore] = scoreCurrent;
                indexStore ++;
            }
            // Store top ten scores
            while(indexStore > SCORE_HIST)
            {
                indexStore--;
                free(topScores[indexStore]);
            }
            config_put_int(_k("%s_scoreLen",scoreCurrent->game),indexStore);
            for(indexMatch=0; indexMatch<indexStore; indexMatch++)
            {
                numTeams = topScores[indexMatch]->num_teams;
                config_put_int(_k("%s_score%dteamNum",scoreCurrent->game,indexMatch+1),numTeams);
                for(indexTeam=0; indexTeam<numTeams; indexTeam++)
                {

                    config_put_string(_k("%s_score%dteam%d",scoreCurrent->game,indexMatch+1,indexTeam+1),
                            score_getTeam(topScores[indexMatch], indexTeam));
                    config_put_int(_k("%s_score%dresult%d",scoreCurrent->game,indexMatch+1,indexTeam+1),
                            topScores[indexMatch]->teamScore[indexTeam]);
                }
            }
            for(indexMatch=0; indexMatch<indexStore; indexMatch++)
            {
                free(topScores[indexMatch]);
            }
        }
        config_save();
    }

    scoreCurrent = NULL;
    return;
}


uint16_t score_getNumTeams(void)
{
    if(scoreCurrent)
    {
        return scoreCurrent->num_teams;
    }
    return 0;
}
/**
 *  Return score of team
 *  @param team Team to return 0...
 *
 */
int32_t score_getScore(int16_t team, char **TeamName)
{
    if(scoreCurrent)
    {
        if((team>=0)&&(team < scoreCurrent->num_teams))
        {
            if(TeamName != NULL)
            {
                *TeamName = score_getTeam(scoreCurrent, team);
            }
            return scoreCurrent->teamScore[team];
        }
    }
    return -1;
}



/**
 *  Return Name of team
 *  @param team Team to return 0...
 *
 */
char *score_getTeam(scoreGame_t* scoreObj, uint16_t team)
{
    char *teamName;
    uint16_t counter;
    if(scoreObj)
    {
        if(team < scoreObj->num_teams)
        {
            teamName = scoreObj->teamNames;
            for(counter = 0;  counter<team; counter++)
            {
                teamName += strlen(teamName) + 1;
            }
            return teamName;
        }
    }
    return NULL;
}




char *score_getGame(void)
{
    if(scoreCurrent)
    {
        return scoreCurrent->game;
    }
    return NULL;
}

/**
 * Retrieve historical data
 */

uint16_t score_H_getNumGames(void)
{
    int32_t index=0;
    config_get_int("scoreGameNum",&index);
    return (uint16_t)index;
}

/**
 * Returns the name of the game and the number of matches
 */
uint16_t score_H_getGameStats(uint16_t game, char*GameName)
{
    int32_t numMatches=0;
    char gameName[500];
    gameName[0]='\0';
    config_get_string(_k("scoreGameTitle%d",game+1),gameName);

    config_get_int(_k("%s_scoreLen",gameName),&numMatches);

    if(GameName != NULL)
    {
        strcpy(GameName, gameName);
    }
    return (uint16_t)numMatches;
}


uint16_t score_H_getNumTeams(uint16_t game, uint16_t match)
{
    int32_t numTeams=0;
    char gameName[500];
    gameName[0]='\0';
    config_get_string(_k("scoreGameTitle%d",game+1),gameName);

    config_get_int(_k("%s_score%dteamNum",gameName,match+1),&numTeams);
    return (uint16_t)numTeams;
}

/**
 * Gets the Team namd and Team score for a match
 * @param game The game index to retrieve details for
 * @param match The match index to retrieve details for
 * @param team The team index to retrieve details for
 * @param [out] Pointer to buffer to recive team name
 *
 * @return The score for this team
 */
int32_t  score_H_getScore(uint16_t game, uint16_t match, uint16_t team, char *TeamName)
{
    int32_t teamScore=0;
    char gameName[500];
    char teamName[500];
    gameName[0]='\0';
    teamName[0]='\0';
    config_get_string(_k("scoreGameTitle%d",game+1),gameName);

    config_get_string(_k("%s_score%dteam%d",gameName,match+1,team+1),teamName);
    config_get_int(_k("%s_score%dresult%d",gameName,match+1,team+1),&teamScore);
    if(TeamName != NULL)
    {
        strcpy(TeamName, teamName);
    }
    return (uint16_t)teamScore;
}

