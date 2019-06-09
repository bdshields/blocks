/*
 * utils.c
 *
 *  Created on: 5 May 2019
 *      Author: brandon
 */


#include <time.h>
#include <stdarg.h>
#include "utils.h"

systime get_systime(void)
{
    systime time;
    struct timezone tz = {0,0};

    gettimeofday(&time, &tz);
    return time;
}


systime set_alarm(uint32_t milliseconds)
{
    systime time;
    time = get_systime();
    time.tv_usec += milliseconds * 1000;
    if(time.tv_usec > 1000000)
    {
        while(time.tv_usec > 1000000)
        {
            time.tv_sec ++;
            time.tv_usec -= 1000000;
        }
    }
    return time;
}

systime cancel_alarm(systime *alarm)
{
    systime new_alarm;
    new_alarm = get_systime(); // Just set it 1 year into the future
    new_alarm.tv_sec += 31536000;
    if(alarm)
    {
        *alarm = new_alarm;
    }
    return new_alarm;
}

uint16_t alarm_expired(systime alarm)
{
    systime cur;

    cur = get_systime();
    if(alarm.tv_sec < cur.tv_sec)
    {
        return 1;
    }
    else if(alarm.tv_sec > cur.tv_sec)
    {
        return 0;
    }
    else if(alarm.tv_usec < cur.tv_usec)
    {
        // seconds are equal, so check micro seconds
        return 1;
    }
    else
    {
        return 0;
    }
}

void frame_sleep(uint32_t ms)
{
    struct timespec requested_time;
    struct timespec remaining;
    requested_time.tv_nsec = (ms%1000)*1000000;
    requested_time.tv_sec = ms/1000;
    nanosleep(&requested_time, &remaining);
}

