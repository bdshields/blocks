/*
 * input.h
 *
 *  Created on: 30 Apr. 2019
 *      Author: brandon
 */

#ifndef SRC_INPUT_H_
#define SRC_INPUT_H_

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

#define MAX_USERS 2

#define INPUT(_user,_button) ((user_input_t){.user=_user, .button=_button})

user_input_t in_get_bu(void);
void in_push(user_input_t input);

void in_init();
void in_close();


#endif /* SRC_INPUT_H_ */
