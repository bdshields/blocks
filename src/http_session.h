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


#define MAX_SESSIONS 4
#define SESSION_TIMEOUT (30)  // timeout in seconds
#define ID_LEN 100

typedef struct http_ses_s{
    char    id[ID_LEN];
    int     player;
    systime timeout;
}http_ses_t;

void http_session_init(void);
http_ses_t *http_session_find(char *id);
void http_session_expire(char *id);


#endif /* SRC_HTTP_SESSION_H_ */
