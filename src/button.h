/*
 * button.h
 *
 *  Created on: 18 Aug 2020
 *      Author: brandon
 */

#ifndef SRC_BUTTON_H_
#define SRC_BUTTON_H_

#include <stdint.h>

typedef enum bu_nav_e{
    bu_none =   0x00,
    bu_up =     0x01,
    bu_down =   0x02,
    bu_left =   0x04,
    bu_right =  0x08,
    bu_a =      0x10,
    bu_b =      0x20,
    bu_start =   0x40,
    bu_select =  0x80,
    // Any more and we need to check code that uses uint8_t
}bu_nav_t;

typedef struct user_input_s {
    int16_t  user;
    bu_nav_t button;
}user_input_t;

#define MAX_USERS 5 // 4 players, plus wildcard

#define INPUT(_user,_button) ((user_input_t){.user=_user, .button=_button})

void button_init();
user_input_t in_get_bu(void);
void in_push(user_input_t input);



#endif /* SRC_BUTTON_H_ */
