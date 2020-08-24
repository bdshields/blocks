/*
 * http_session.c
 *
 *  Created on: 28 May 2019
 *      Author: brandon
 */

#include "http_session.h"
#include <string.h>
#include <stddef.h>
#include <stdio.h>

http_ses_t sessions[MAX_SESSIONS];
uint16_t   session_max;

int16_t http_session_FindIdlePlayer(void);
int16_t http_session_FindExpiredPlayer(void);
void http_session_TagInPlayer(int16_t OldPlayer, int16_t NewPlayer);


void http_session_init(void)
{
    uint16_t counter;
    for (counter=0; counter<MAX_SESSIONS; counter++)
    {
        sessions[counter].id[0] = '\0';
    }
    session_max = MAX_SESSIONS;
}

/**
 * Finds a matching session, or returns an allocated session
 */
http_ses_t *http_session_find(char *id)
{
    int16_t idx;
    int16_t  spare = -1;
    int16_t  old = -1;
    int16_t  expiredPlayer = -1;
    int16_t  idlePlayer = -1;
    int16_t  existing = -1;

    // find expired player
    expiredPlayer = http_session_FindExpiredPlayer();
    // find idle player
    idlePlayer = http_session_FindIdlePlayer();


    // look for existing session
    for (idx=0; idx<MAX_SESSIONS; idx++)
    {
        if(strcmp(id, sessions[idx].id) == 0)
        {
            sessions[idx].timeout = set_alarm(SESSION_TIMEOUT * 1000);
            existing = idx;
        }
        else if((spare == -1) && (sessions[idx].id[0] == '\0'))
        {
            spare = idx;
        }
        else if((old == -1) && alarm_expired(sessions[idx].timeout))
        {
            old = idx;
        }
    }
    if(existing != -1)
    {
        if(sessions[existing].player == 0)
        {
            // If existing session is an idle player, add player to game
            http_session_TagInPlayer(expiredPlayer, existing);
        }
        else
        {
            // attempt to add other idle player
            http_session_TagInPlayer(expiredPlayer, idlePlayer);
        }
        return sessions + existing;
    }
    else if( spare != -1)
    {
        idx = spare;
    }
    else if(old != -1)
    {
        idx = old;
    }
    else
    {
        idx = -1;
    }
    if(idx >= 0)
    {
        if(expiredPlayer != -1)
        {
            // New session becomes an active player
            strncpy(sessions[idx].id, id, ID_LEN);
            sessions[idx].timeout = set_alarm(SESSION_TIMEOUT * 1000);
            http_session_TagInPlayer(expiredPlayer, idx);
        }
        else
        {
            // Add new idle player
            strncpy(sessions[idx].id, id, ID_LEN);
            sessions[idx].timeout = set_alarm(SESSION_TIMEOUT * 1000);
            sessions[idx].player = 0;
            sessions[idx].team = 0;
            sessions[idx].name[0] = '\0';
        }
        return sessions + idx;
    }

    return NULL;
}

void http_session_expire(char *id)
{
    int16_t expiredPlayer;
    int16_t idlePlayer;
    // Hand over session if part of team
    //Find idle player

    idlePlayer = http_session_FindIdlePlayer();


    for (expiredPlayer=0; expiredPlayer<MAX_SESSIONS; expiredPlayer++)
    {
        if(strcmp(id, sessions[expiredPlayer].id) == 0)
        {
            // Was this session an active player
            if(sessions[expiredPlayer].player > 0)
            {
                if(idlePlayer != -1)
                {
                    http_session_TagInPlayer(expiredPlayer, idlePlayer);
                }
                else
                {
                    // no idle players, so just expire the timer
                    sessions[expiredPlayer].timeout = set_alarm(0);
                }
            }
            return ;
        }
    }
}


uint16_t http_session_setPlayers(uint16_t numPlayers)
{
    int16_t idx;
    uint16_t count=0;
    for (idx=0; idx<MAX_SESSIONS; idx++)
    {
        if((sessions[idx].id[0] != '\0')
                && (! alarm_expired(sessions[idx].timeout)))
        {
            count++;
            sessions[idx].player = count;
            if(sessions[idx].name[0] == '\0')
            {
                sprintf(sessions[idx].name,"Player %d",sessions[idx].player);
            }
            if(count >= numPlayers)
            {
                break;
            }
        }
    }
    return count;
}

void http_session_clrPlayers(void)
{
    int16_t idx;
    for (idx=0; idx<MAX_SESSIONS; idx++)
    {
        sessions[idx].player = 0;
        sessions[idx].team = 0;
        sessions[idx].name[0] = '\0';
        if(alarm_expired(sessions[idx].timeout))
        {
            sessions[idx].id[0] = '\0';
        }
    }
}

uint16_t http_session_setTeams(uint16_t teamSize)
{
    int16_t idx;
    uint16_t count=0;
    uint16_t team=1;
    for (idx=0; idx<MAX_SESSIONS; idx++)
    {
        if(sessions[idx].player != 0)
        {
            count++;
            sessions[idx].team = team;
            if(count >= teamSize)
            {
                team++;
                count=0;
            }
        }
    }
    return team;
}

char *http_session_getTeamName(uint16_t team)
{
    int16_t idx;
    if(team == 0)
    {
        return NULL;
    }
    for (idx=0; idx<MAX_SESSIONS; idx++)
    {
        if(sessions[idx].team == team)
        {
            return sessions[idx].name;
        }
    }
    return NULL;
}

uint16_t http_session_countActive(void)
{
    int16_t idx;
    uint16_t count=0;
    for (idx=0; idx<MAX_SESSIONS; idx++)
    {
        if((sessions[idx].id[0] != '\0')
                && (! alarm_expired(sessions[idx].timeout)))
        {
            count++;
        }
    }
    return count;
}


int16_t http_session_FindExpiredPlayer(void)
{
    int16_t idx;

    for (idx=0; idx<MAX_SESSIONS; idx++)
    {
        if((sessions[idx].id[0] != '\0') && alarm_expired(sessions[idx].timeout))
        {
            // Was this session an active player
            if((sessions[idx].player > 0))
            {
                return idx;
            }
        }
    }
    return -1;
}

int16_t http_session_FindIdlePlayer(void)
{
    int16_t idx;
    //Find idle player
    for (idx=0; idx<MAX_SESSIONS; idx++)
    {
        if( (sessions[idx].id[0] != '\0') && (! alarm_expired(sessions[idx].timeout) ) )
        {
            if(sessions[idx].player == 0)
            {
                return idx;
            }
        }
    }
    return -1;

}

void http_session_TagInPlayer(int16_t OldPlayer, int16_t NewPlayer)
{
    if((OldPlayer > -1) && (NewPlayer > -1))
    {
        // tag in an idle player
        sessions[NewPlayer].player = sessions[OldPlayer].player;
        sessions[NewPlayer].team = sessions[OldPlayer].team;
        strcpy(sessions[NewPlayer].name, sessions[OldPlayer].name);

        sessions[OldPlayer].id[0] = '\0';
    }
}
