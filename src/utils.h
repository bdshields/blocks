/*
 * utils.h
 *
 *  Created on: 5 May 2019
 *      Author: brandon
 */

#ifndef SRC_UTILS_H_
#define SRC_UTILS_H_

#include <sys/time.h>
#include <stdint.h>

typedef struct timeval systime;
systime get_systime(void);
systime set_alarm(uint32_t milliseconds);
systime cancel_alarm(systime *alarm);

uint16_t alarm_expired(systime alarm);


void frame_sleep(uint32_t ms);

#endif /* SRC_UTILS_H_ */
