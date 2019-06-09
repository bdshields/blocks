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

void http_session_init(void)
{
    uint16_t counter;
    for (counter=0; counter<MAX_SESSIONS; counter++)
    {
        sessions[counter].id[0] = '\0';
    }
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
        sessions[idx].player = idx + 1;
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



