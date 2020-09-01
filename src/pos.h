/*
 * pos.h
 *
 *  Created on: 30 Apr. 2019
 *      Author: brandon
 */

#ifndef SRC_POS_H_
#define SRC_POS_H_

#include <stdint.h>

typedef struct pos_s{
    int16_t x;
    int16_t y;
}pos_t;

typedef enum transform_e{
    tr_none,
    tr_rot_left,
    tr_rot_right,
}transform_t;


typedef enum dir_e{
    none  = 0x00,
    up =    0x01,
    down =  0x02,
    left =  0x04,
    right = 0x08,
}dir_t;


dir_t pos_is_adjacent(pos_t* location, pos_t* position);
pos_t  pos_rotate(pos_t position, pos_t origin, transform_t rotate);
int16_t pos_equal(pos_t pos, int16_t x, int16_t y);
pos_t pos_add(pos_t pos1, pos_t pos2);


#endif /* SRC_POS_H_ */
