/*
 * pos.c
 *
 *  Created on: 30 Apr. 2019
 *      Author: brandon
 */

#include "pos.h"

/**
 * returns direction is position is adjacent to location
 */
dir_t pos_is_adjacent(pos_t* location, pos_t* position)
{
    dir_t direction = none;
    if(position->x == location->x-1)
    {
        direction = left;
    }
    else if(position->x == location->x+1)
    {
        direction = right;
    }
    else if(position->y == location->y+1)
    {
        direction = down;
    }
    else if(position->y == location->y-1)
    {
        direction = up;
    }

    return direction;
}



/**
 * returns a position that has been rotated
 *
 * Generalise rotation matrix
 * [ Cos(theta), -Sin(theta)]
 * [ Sin(theta),Cos(theta)]
 *
 *  theta is angle from x axis (i.e. counter clockwise)
 *  https://en.wikipedia.org/wiki/Rotation_matrix
 */

const int16_t left_rot[2][2]={{0, -1},
                               {1, 0}};
const int16_t right_rot[2][2]={{0,  1},
                              {-1, 0}};

pos_t  pos_rotate(pos_t position, pos_t origin, transform_t rotate)
{
    // conduct matrix multiply
    pos_t output;

    // move subject to origin
    position.x = (position.x << 1) - origin.x;
    position.y = (position.y << 1) - origin.y;

    // conduct rotation
    switch(rotate)
    {
    case tr_rot_right:
        output.x = position.x*left_rot[0][0] + position.y*left_rot[0][1];
        output.y = position.x*left_rot[1][0] + position.y*left_rot[1][1];
        break;
    case tr_rot_left:
        output.x = position.x*right_rot[0][0] + position.y*right_rot[0][1];
        output.y = position.x*right_rot[1][0] + position.y*right_rot[1][1];
        break;
    default:
        output = position;
        break;
    }

    // move subject back into position
    output.x = (output.x + origin.x) >> 1;
    output.y = (output.y + origin.y) >> 1;

    return output;
}

int16_t pos_equal(pos_t pos_1, pos_t pos_2)
{
    if((pos_1.x == pos_2.x) && (pos_1.y == pos_2.y))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

pos_t pos_add(pos_t pos1, pos_t pos2)
{
    pos1.x += pos2.x;
    pos1.y += pos2.y;

    return pos1;
}

pos_t pos_subtract(pos_t pos1, pos_t pos2)
{
    pos1.x -= pos2.x;
    pos1.y -= pos2.y;

    return pos1;
}

