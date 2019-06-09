/*
 * input.c
 *
 *  Created on: 30 Apr. 2019
 *      Author: brandon
 */

#include <stdio.h>
#include <stdint.h>
#include <termios.h>
#include <unistd.h>
#include <pthread.h>
#include "input.h"

void *input_main(void* context);
pthread_t input_thread;
struct termios terminal_attr;



bu_nav_t in_button[MAX_USERS] = {bu_none};
bu_nav_t in_button_buffered[MAX_USERS] ={bu_none};
pthread_mutex_t    in_button_mtex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Returns one button press at a time, from one user at a time
 */
user_input_t in_get_bu(void)
{
    static uint16_t user_toggle=0;

    user_input_t input;
    uint8_t  mask;

    input.user = -1;
    input.button = bu_none;

    // Buffer the button inputs so that there are no priority issues
    if(in_button_buffered[user_toggle] == bu_none)
    {
        if(pthread_mutex_trylock(&in_button_mtex) == 0)
        {
            in_button_buffered[user_toggle] = in_button[user_toggle];
            in_button[user_toggle] = bu_none;
            pthread_mutex_unlock(&in_button_mtex);
        }
    }
    // Test each button bit and return when we find one
    mask = 0x80;
    while(mask > 0)
    {
        if(in_button_buffered[user_toggle] & mask)
        {
            // Button is set, so return it
            input.user = user_toggle;
            input.button = mask;
            in_button_buffered[user_toggle] &= ~mask;
            break;
        }
        mask >>=1;
    }
    // Prepare to check next user
    user_toggle ++;
    if(user_toggle >= MAX_USERS)
    {
        user_toggle = 0;
    }
    // Return the data
    return input;
}

/**
 *  Register a button press
 */
void in_push(user_input_t input)
{
    struct timespec giveup_time;
    giveup_time.tv_sec = 0;
    giveup_time.tv_nsec = 500000000;

    if(input.user < MAX_USERS)
    {
        if(pthread_mutex_timedlock(&in_button_mtex, &giveup_time) == 0)
        {
            in_button[input.user] |= input.button;
            pthread_mutex_unlock(&in_button_mtex);
        }
    }
}


/**
 * Initialise the input system
 */
void in_init()
{
    struct termios tattr;

    tcgetattr (STDIN_FILENO, &terminal_attr);
    tcgetattr (STDIN_FILENO, &tattr);
    tattr.c_lflag &= ~(ICANON|ECHO); /* Clear ICANON and ECHO. */
    tattr.c_cc[VMIN] = 1;
    tattr.c_cc[VTIME] = 0;
    tcsetattr (STDIN_FILENO, TCSAFLUSH, &tattr);
    pthread_mutex_init(&in_button_mtex,NULL);
    pthread_create( &input_thread, NULL, input_main, (void*) 0);
}

void in_close()
{
    tcsetattr (STDIN_FILENO, TCSAFLUSH, &terminal_attr);
}

void *input_main(void* context)
{
    int data;
    while(1)
    {
        data = getchar();
        switch(data)
        {
        case 0x1b: // Terminal escape sequence
            data = getchar(); // discard '['
            data = getchar();
            switch(data)
            {
            case 'A':
                // Up arrow
                in_push(INPUT(0,bu_up));
                break;
            case 'B':
                // Down arrow
                in_push(INPUT(0,bu_down));
                break;
            case 'C':
                // Right arrow
                in_push(INPUT(0,bu_right));
                break;
            case 'D':
                // Left arrow
                in_push(INPUT(0,bu_left));
                break;
            }
            printf("\x1b[s\x1b[0;0HReceived Esc:  %3d                   \x1b[u",data);
            break;
        default:
            switch(data)
            {
            /**
             *   PLAYER   1
             */
            case '.':
                in_push(INPUT(0,bu_a));
                break;
            case ',':
                in_push(INPUT(0,bu_b));
                break;
            case ']':
                in_push(INPUT(0,bu_start));
                break;
            case '[':
                in_push(INPUT(0,bu_select));
                break;
                /**
                 *   PLAYER   2
                 */
            case 'w':
                // Up arrow
                in_push(INPUT(1,bu_up));
                break;
            case 's':
                // Down arrow
                in_push(INPUT(1,bu_down));
                break;
            case 'd':
                // Right arrow
                in_push(INPUT(1,bu_right));
                break;
            case 'a':
                // Left arrow
                in_push(INPUT(1,bu_left));
                break;
            case 'b':
                in_push(INPUT(1,bu_a));
                break;
            case 'v':
                in_push(INPUT(1,bu_b));
                break;
            case 'r':
                in_push(INPUT(1,bu_start));
                break;
            case 't':
                in_push(INPUT(1,bu_select));
                break;
            }
            printf("\x1b[s\x1b[0;0HReceived data: %3d                   \x1b[u",data);
            break;
        }
    }
}



