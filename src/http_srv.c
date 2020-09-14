/*
 * http_srv.c
 *
 *  Created on: 6 May 2019
 *      Author: brandon
 */


#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>

#include "html.h"
#include "utils.h"
#include "button.h"
#include "http_session.h"
#include "http_srv.h"
#include "json.h"
#include "scoring.h"

#include "debug.h"

#define DEBUG_LEVEL DBG_INFO

pthread_t http_thread;

void *http_main(void* context);
int http_proc(int fd);
char * http_parse_header(uint16_t *len);

uint16_t http_close;
#define HTTP_RESP_SIZE 10000

int http_respond(int fd, uint16_t type, char *resource);

void http_init(void)
{
    http_close = 0;
    http_session_init();
    pthread_create( &http_thread, NULL, http_main, (void*) 0);
}

void http_shutdown(void)
{
    systime timeout;
    http_close = 1; // request shutdown
    timeout = set_alarm(5000);
    DEBUG("Requesting shutdown\n");
    pthread_kill( http_thread, SIGUSR1);
    while((alarm_expired(timeout) == 0) && (http_close != 2))
    {
        // wait for shutdown complete
        frame_sleep(100);
    }
    DEBUG("Shutdown complete\n");

}

void http_usr1(int sig)
{
    return;
}

void *http_main(void* context)
{
    int http_socket;
    struct sockaddr_in name;
    fd_set active_fd_set;
    fd_set read_fd_set;
    int i;

    struct sockaddr_in clientname;
    size_t size;

    signal (SIGUSR1,http_usr1);

    // Init socket
    http_socket = socket (PF_INET, SOCK_STREAM, 0);
    name.sin_family = AF_INET;
    name.sin_port = htons (HTTP_SRV_PORT);
    name.sin_addr.s_addr = htonl (INADDR_ANY);

    DEBUG("Starting Server\n");
    while(bind (http_socket, (struct sockaddr *) &name, sizeof (name)) != 0)
    {
        // Failed to set port, try again in 1 second
        if(http_close)
        {
            goto close_http;
        }
        frame_sleep(1000);
    }

    // setup listening
    while(listen (http_socket, 50)!=0)
    {
        // Failed to listen, try again in 1 second
        if(http_close)
        {
            goto close_http;
        }
        frame_sleep(1000);
    }

    DEBUG("Server Ready\n");

    FD_ZERO (&active_fd_set);
    FD_SET (http_socket, &active_fd_set);

    while (1)
    {
        /* Block until input arrives on one or more active sockets. */
        read_fd_set = active_fd_set;
        if (select (FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0)
        {
            if(errno == EINTR)
            {
                DEBUG("requested to shutdown\n");
            }
            else
            {
                DEBUG("select returned error %d\n", errno);
            }
            goto close_http;
        }
        if(http_close)
        {
            DEBUG("shutting down\n");
            goto close_http;
        }

        /* Service all the sockets with input pending. */
        for (i = 0; i < FD_SETSIZE; ++i)
        {
            if (FD_ISSET (i, &read_fd_set))
            {
                if (i == http_socket)
                {
                    /* Connection request on original socket. */
                    int new;
                    size = sizeof (clientname);
                    new = accept (http_socket,(struct sockaddr *) &clientname, &size);
                    if (new < 0)
                    {
                        DEBUG("Error accepting new connection, Exiting\n");
                        http_close = 2;
                        exit (0);
                    }
                    else
                    {
                        DEBUG("New connection: %s:%hu\n", inet_ntoa(clientname.sin_addr), clientname.sin_port);
                    }
                    FD_SET (new, &active_fd_set);
                }
                else
                {
                    int result;
                    /* Data arriving on an already-connected socket. */
                    if ((result = http_proc (i)) < 0)
                    {
                        DEBUG("Closing socket\n");
                        close (i);
                        FD_CLR (i, &active_fd_set);
                    }
                }
            }
        }
    }
close_http:
    if(http_socket > -1)
    {
        for (i = 0; i < FD_SETSIZE; ++i)
        {
            if (FD_ISSET (i, &active_fd_set))
            {
                shutdown(i, 2);
                close(i);
                FD_CLR (i, &active_fd_set);
            }
        }
        shutdown(http_socket, 2);
        close(http_socket);
    }
    http_close = 2;
    DEBUG("http_srv: Exiting\n");
    return 0;
}

INCHTML(remote,"../src/remote.html");
INCHTML(scoreboard,"../src/scoreboard.html");
INCHTML(config,"../src/config.html");


typedef struct player_status_s{
    char        *id;
    char        *name;
    bu_nav_t    buttons;
    int8_t      action;  //0 - None, 1 - Update, 2 - expire
}player_status_t;
void player_json_callback(char *key, char *value, void *param);

int http_proc(int fd)
{
    char buffer[HTTP_RESP_SIZE];
    int  length;
    http_ses_t *session;
    length = read (fd, buffer, HTTP_RESP_SIZE);

    if(length < 0)
    {
        DEBUG("Error (%d) reading from socket\n", errno);
        return -1;
    }
    else if(length == 0)
    {
        return -1;
    }
    else
    {
        // parse our message
        uint16_t payload_len;
        char *verb;
        char *path;
        char *form;

        buffer[length] = 0;
        DEBUG("HTTP REQUEST:\n%s\n", buffer);
        verb = strtok(buffer," "); // Get Verb
        path = strtok(NULL," ");   // Get Path
        strtok(NULL,"\n");         // Drop the HTTP version
        form = http_parse_header(&payload_len);
        if((form + payload_len) > (buffer + length))
        {
            while((form + payload_len) > (buffer + length))
            {
                int more_read;
                more_read = read (fd, buffer+length, HTTP_RESP_SIZE);
                if(more_read > 0)
                {
                    DEBUG("Reading more...\n");
                    length += more_read;
                }
                else if(more_read < 0){
                    DEBUG("Read result %d, Error (%d)\n", more_read, errno);
                    return -1;
                }
            }
            buffer[length] = 0;
            DEBUG("Payload:\n%s\n", form);
        }
        if(strcmp("GET", verb) == 0)
        {
            if(strcmp("/", path) == 0)
            {
                http_respond(fd,200,(char*)&html_remote_start);
            }
            else if(strcmp("/config", path) == 0)
            {
                http_respond(fd,200,(char*)&html_config_start);
            }
            else if(strcmp("/scoreboard", path) == 0)
            {
                http_respond(fd,200,(char*)&html_scoreboard_start);
            }
            else if(strcmp("/score", path) == 0)
            {
                char        buffer[HTTP_RESP_SIZE];
                uint16_t    pos;
                uint16_t    player;
                char        *teamName;
                int32_t      teamScore;
                pos = 0;
                player = 0;
                pos += sprintf(buffer+pos,"{ \"type\":\"scoreboard\", \"data\": [");
                if(score_getNumTeams() > 0)
                {
                    // print first entry
                    teamScore = score_getScore(player, &teamName);
                    pos += sprintf(buffer+pos,"{ \"game\": \"%s\", \"matches\":[ {\"result\":[{\"team\": \"%s\", \"score\": %d }\n", score_getGame(),teamName, teamScore);
                    player++;
                    while(player < score_getNumTeams())
                    {
                        teamScore = score_getScore(player, &teamName);
                        pos += sprintf(buffer+pos,",{ \"team\": \"%s\", \"score\": %d }\n", teamName, teamScore);
                        player++;
                    }
                    pos += sprintf(buffer+pos,"]}]}]}");
                }
                else
                {
                    // Generate leader board
                    char tempString[500];
                    char     comma;
                    uint16_t numGames;
                    uint16_t numResults;
                    uint16_t numTeams;
                    uint16_t idxGames;
                    uint16_t idxResults;
                    uint16_t idxTeams;
                    int32_t  score;

                    // print first entry
                    numGames = score_H_getNumGames();
                    comma = ' ';
                    for(idxGames = 0; idxGames<numGames; idxGames++)
                    {
                        numResults = score_H_getGameStats(idxGames, tempString);
                        pos += sprintf(buffer+pos,"%c{ \"game\": \"%s\", \"matches\":[ ",comma, tempString);
                        comma = ' ';
                        for(idxResults=0; idxResults<numResults;idxResults++)
                        {
                            pos += sprintf(buffer+pos,"%c{\"result\":[", comma);
                            numTeams = score_H_getNumTeams(idxGames, idxResults);
                            comma = ' ';
                            for(idxTeams=0; idxTeams<numTeams;idxTeams++)
                            {
                                score = score_H_getScore(idxGames, idxResults, idxTeams, tempString);
                                pos += sprintf(buffer+pos,"%c{ \"team\": \"%s\", \"score\": %d}",comma, tempString, score);
                                comma = ',';
                            }
                            pos += sprintf(buffer+pos,"]}");
                            comma = ',';

                        }
                        pos += sprintf(buffer+pos,"]}");
                        comma = ',';

                    }
                    pos += sprintf(buffer+pos,"]}");

                }
                http_respond(fd,200,buffer);
            }
        }
        else if(strcmp("POST", verb) == 0)
        {
            if(strcmp("/player", path) == 0)
            {
                player_status_t status={0};
                // skip header
                json_parse_object(form, player_json_callback, &status);
                if(status.action == 1)
                {
                    // update player status
                    session=http_session_find( status.id);
                    if(session != NULL)
                    {
                        char response[500];
                        if(strcmp(status.name,"Unknown") != 0)
                        {
                            // Update player's name
                            strcpy(session->name,status.name);
                        }
                        in_push((user_input_t){session->player,status.buttons} );
                        snprintf(response, 200,"{\"player\": \"%d\", \"score\": %d}",session->player, score_getScore(session->team-1, NULL));
                        http_respond(fd,200,response);
                    }
                    else
                    {
                        char response[100];
                        snprintf(response, 100,"{\"player\": \"?\", \"score\": 0}");
                        http_respond(fd,200,response);
                    }
                }
                else if(status.action == 2)
                {
                    // Player exiting
                    http_session_expire( status.id);
                    http_respond(fd,204,NULL);
                }
            }
        }
    }
    return 0;
}

#define HTTP_200(_func1, _func2)\
        _func1("HTTP/1.1 200 OK\r\n") \
        _func1("Content-Type: text/html; charset=UTF-8\r\n")\
        _func2("Content-Length: %d\r\n",length) \
        _func1("Server: blocks/1.0.0 (Linux) (raspbian/Linux)\r\n") \
        _func1("Accept-Ranges: none\r\n") \
        _func1("\r\n") \
        _func1("%s")

#define HTTP_204(_func1)\
        _func1("HTTP/1.1 204 OK\r\n") \
        _func1("Server: blocks/1.0.0 (Linux) (raspbian/Linux)\r\n") \
        _func1("\r\n")



int http_respond(int fd, uint16_t type, char *resource)
{
    uint16_t length;
    char buffer[HTTP_RESP_SIZE];
    int resp_len;
    if(resource)
    {
        length = strlen(resource);
    }
    else
    {
        resource = "";
        length = 0;
    }

    if(type == 200)
    {
#define _str1(_a) _a
#define _str2(_a,_b) _a
#define _var1(_a)
#define _var2(_a,_b) _b,

        resp_len = snprintf(buffer,HTTP_RESP_SIZE,
                HTTP_200(_str1, _str2),
                HTTP_200(_var1, _var2)resource
        );
        write(fd, buffer, resp_len);
#undef _str1
#undef _str2
#undef _var1
#undef _var2

    }
    else if(type == 204)
    {
#define _str1(_a) _a

        resp_len = snprintf(buffer,HTTP_RESP_SIZE,
                HTTP_204(_str1)
        );
        write(fd, buffer, resp_len);
#undef _str1

    }
    return 0;
}

/**
 * return length of payload
 */
char * http_parse_header(uint16_t *len)
{
    uint16_t length = 0;
    char *key;
    char *value;
    while(1)
    {
        key = strtok(NULL,":\n");  // Look for a key or a new line
        if(key[0] == '\r')
        {
            // Reached the end of the header
            break;
        }
        value  = strtok(NULL,"\n");
        if(strcmp(key,"Content-Length") == 0)
        {
            length = strtol(value,NULL,10);
        }
    }
    if(len)
    {
        *len = length;
    }
    key += 2;
    return key;
}


void player_json_callback(char *key, char *value, void *param)
{
    player_status_t *status;
    status = (player_status_t*)param;
    if(strcmp(key,"id") == 0)
    {
        status->id = value;
    }
    else if(strcmp(key,"buttons") == 0)
    {
        status->buttons = 0;
        value = strtok(value,",\0");
        while(value != NULL)
        {
            if(strstr(value,"up") != NULL)
            {
                status->buttons |= bu_up;
            }
            else if(strstr(value,"down") != NULL)
            {
                status->buttons |= bu_down;
            }
            else if(strstr(value,"left") != NULL)
            {
                status->buttons |= bu_left;
            }
            else if(strstr(value,"right") != NULL)
            {
                status->buttons |= bu_right;
            }
            else if(strstr(value,"start") != NULL)
            {
                status->buttons |= bu_start;
            }
            else if(strstr(value,"select") != NULL)
            {
                status->buttons |= bu_select;
            }
            else if(strstr(value,"a") != NULL)
            {
                status->buttons |= bu_a;
            }
            else if(strstr(value,"b") != NULL)
            {
                status->buttons |= bu_b;
            }
            value = strtok(NULL,",\0");
        }
    }
    else if(strcmp(key,"name") == 0)
    {
        status->name = value;
    }
    else if(strcmp(key,"action") == 0)
    {
        if(strcmp(value,"update") == 0)
        {
            status->action = 1;
        }
        else if(strcmp(value,"expire") == 0)
        {
            status->action = 2;
        }
    }
}

