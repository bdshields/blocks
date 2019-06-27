/*
 * http_session.c
 *
 *  Created on: 28 May 2019
 *      Author: brandon
 */

#include "http_session.h"
#include <string.h>
#include <stddef.h>

http_ses_t sessions[MAX_SESSIONS];
uint16_t   session_max;

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
    // look for existing session
    for (idx=0; idx<MAX_SESSIONS; idx++)
    {
        if(strcmp(id, sessions[idx].id) == 0)
        {
            sessions[idx].timeout = set_alarm(SESSION_TIMEOUT * 1000);
            return sessions + idx;
        }
        else if((spare == -1) && (sessions[idx].id[0] == '\0'))
        {
            spare = idx;
        }
        else if((spare == -1)&&(alarm_expired(sessions[idx].timeout)))
        {
            spare = idx;
        }
    }
    // allocate one. Prioritise spare over old.
    if( spare != -1)
    {
        idx = spare;
    }
    else
    {
        idx = -1;
    }
    if(idx >= 0)
    {
        strncpy(sessions[idx].id, id, ID_LEN);
        sessions[idx].timeout = set_alarm(SESSION_TIMEOUT * 1000);
        sessions[idx].player = 0;
        sessions[idx].team = 0;
        sessions[idx].name[0] = '\0';
        return sessions + idx;
    }

    return NULL;
}

void http_session_expire(char *id)
{
    int16_t idx;
    for (idx=0; idx<MAX_SESSIONS; idx++)
    {
        if(strcmp(id, sessions[idx].id) == 0)
        {
            sessions[idx].id[0] = '\0';
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
        if(sessions[idx].id[0] != '\0')
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
        if(sessions[idx].id[0] != '\0')
        {
            count++;
        }
    }
    return count;
}



