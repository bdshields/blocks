/*
 * http_session.h
 *
 *  Created on: 28 May 2019
 *      Author: brandon
 */

#ifndef SRC_HTTP_SESSION_H_
#define SRC_HTTP_SESSION_H_

#include <stdint.h>

#include "utils.h"


#define MAX_SESSIONS 10
#define SESSION_TIMEOUT (30)  // timeout in seconds
#define ID_LEN 100

typedef struct http_ses_s{
    char    id[ID_LEN];
    char    name[500];
    int     player;  // 0: Non player, 1..4 Active player
    int     team;    // 0 == No team
    systime timeout;
}http_ses_t;

void http_session_init(void);
http_ses_t *http_session_find(char *id);
void http_session_expire(char *id);

uint16_t http_session_setPlayers(uint16_t numPlayers);
void http_session_clrPlayers(void);
uint16_t http_session_setTeams(uint16_t teamSize);
uint16_t http_session_countActive(void);
char *http_session_getTeamName(uint16_t team);


#endif /* SRC_HTTP_SESSION_H_ */
