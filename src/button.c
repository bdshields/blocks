/*
 * button.c
 *
 *  Created on: 18 Aug 2020
 *      Author: brandon
 */
#include "button.h"
#include <pthread.h>


bu_nav_t in_button[MAX_USERS] = {bu_none};
bu_nav_t in_button_buffered[MAX_USERS] ={bu_none};
pthread_mutex_t    in_button_mtex = PTHREAD_MUTEX_INITIALIZER;


void button_init()
{
    pthread_mutex_init(&in_button_mtex,NULL);

}

/**
 * Returns one button press at a time, from one user at a time
 */
user_input_t in_get_bu(void)
{
    static uint16_t last_user = 0;
    uint16_t user;

    user_input_t input;
    uint8_t  mask;

    input.user = -1;
    input.button = bu_none;

    // Buffer the button inputs so that there are no priority issues
    for(user = 0; user<MAX_USERS;  user++)
    {
        if(in_button_buffered[user] == bu_none)
        {
            if(pthread_mutex_trylock(&in_button_mtex) == 0)
            {
                in_button_buffered[user] = in_button[user];
                in_button[user] = bu_none;
                pthread_mutex_unlock(&in_button_mtex);
            }
        }
    }
    // Test each button bit and return when we find one
    user = last_user;
    do{
        user ++;
        if(user == MAX_USERS)
        {
            user = 0;
        }
        mask = 0x80;
        while(mask > 0)
        {
            if(in_button_buffered[user] & mask)
            {
                // Button is set, so return it
                input.user = user;
                input.button = mask;
                in_button_buffered[user] &= ~mask;
                goto found_button;
            }
            mask >>=1;
        }
    }while(user != last_user);
found_button:
    last_user = user;
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



