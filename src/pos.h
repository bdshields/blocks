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

typedef pos_t origin_t;  // Same as pos_t but has half increments i.e. origin == 2*pos

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

#define POS(_x,_y) (pos_t){_x,_y}
#define ORIGIN(_x,_y) (origin_t){(int)(_x*2.0),(int)(_y*2.0)}
#define ORI_2_POS(_origin) (pos_t){_origin.x>>1, _origin.y>>1}
//#define ORI_2_POS(_origin) (pos_t){0, 0}

dir_t pos_is_adjacent(pos_t* location, pos_t* position);
pos_t  pos_rotate(pos_t position, pos_t origin, transform_t rotate);
int16_t pos_equal(pos_t pos_1, pos_t pos_2);
pos_t pos_add(pos_t pos1, pos_t pos2);
pos_t pos_subtract(pos_t pos1, pos_t pos2);


#endif /* SRC_POS_H_ */
