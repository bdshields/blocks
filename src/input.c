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
#include "button.h"

void *input_main(void* context);
pthread_t input_thread;
struct termios terminal_attr;




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
                in_push(INPUT(1,bu_up));
                break;
            case 'B':
                // Down arrow
                in_push(INPUT(1,bu_down));
                break;
            case 'C':
                // Right arrow
                in_push(INPUT(1,bu_right));
                break;
            case 'D':
                // Left arrow
                in_push(INPUT(1,bu_left));
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
                in_push(INPUT(1,bu_a));
                break;
            case ',':
                in_push(INPUT(1,bu_b));
                break;
            case ']':
                in_push(INPUT(1,bu_start));
                break;
            case '[':
                in_push(INPUT(1,bu_select));
                break;
                /**
                 *   PLAYER   2
                 */
            case 'w':
                // Up arrow
                in_push(INPUT(2,bu_up));
                break;
            case 's':
                // Down arrow
                in_push(INPUT(2,bu_down));
                break;
            case 'd':
                // Right arrow
                in_push(INPUT(2,bu_right));
                break;
            case 'a':
                // Left arrow
                in_push(INPUT(2,bu_left));
                break;
            case 'b':
                in_push(INPUT(2,bu_a));
                break;
            case 'v':
                in_push(INPUT(2,bu_b));
                break;
            case 'r':
                in_push(INPUT(2,bu_start));
                break;
            case 't':
                in_push(INPUT(2,bu_select));
                break;
            }
            printf("\x1b[s\x1b[0;0HReceived data: %3d                   \x1b[u",data);
            break;
        }
    }
}



