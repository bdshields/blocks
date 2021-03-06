/*
 * image_util.c
 *
 *  Created on: 28 Apr. 2019
 *      Author: brandon
 */

#include <string.h>
#include <stdint.h>

#include "image_util.h"
#include "frame_buffer.h"

/**
 * paste a sprite into a frame buffer
 *
 * @return True if entire sprite fits in raster
 */
uint16_t paste_sprite(raster_t *frame_buffer, raster_t *sprite, pos_t pos)
{
    uint16_t x;
    uint16_t y;
    uint16_t x_pos;
    uint16_t y_pos;
    uint16_t touch = none;

    y_pos = pos.y;
    for(y=0; y<sprite->y_max; y++)
    {
        x_pos = pos.x;
        for(x=0; x<sprite->x_max; x++)
        {
            // Copy visible pixels
            if(fb_get_pixel(sprite,x,y)->flags & R_VISIBLE)
            {
                if(     (x_pos >= 0) && (x_pos < frame_buffer->x_max)
                     && (y_pos >= 0) && (y_pos < frame_buffer->y_max))
                {
                    // if pixel is visible
                    memcpy(fb_get_pixel(frame_buffer,x_pos, y_pos), fb_get_pixel(sprite,x,y), sizeof(pixel_t));
                }
            }
            x_pos ++;
        }
        y_pos++;
    }
    return touch;
}


void clear_raster(raster_t *frame_buffer)
{
    memset(frame_buffer->image, 0, IMAGE_SIZE(frame_buffer->x_max, frame_buffer->y_max));
}


uint16_t pos_out_raster(raster_t *raster, pos_t pos)
{
    uint16_t outside = 0;
    if(pos.x < 0)
    {
        outside |= left;
    }
    if(pos.x >= raster->x_max)
    {
        outside |= right;
    }
    if(pos.y < 0)
    {
        outside |= up;
    }
    if(pos.y >= raster->y_max)
    {
        outside |= down;
    }
    return outside;
}

void sprite_parser(raster_t *sprite, uint16_t flag_mask, sp_cb callback_f, void *param1, void *param2, void *param3)
{
    uint16_t x;
    uint16_t y;
    pixel_t *pixel;
    pixel = sprite->image;
    for(y=0; y<sprite->y_max; y++)
    {
        for(x=0; x<sprite->x_max; x++)
        {
            if(pixel->flags & flag_mask)
            {
                callback_f(pixel, x, y, param1, param2, param3);
            }
            pixel ++;
        }
    }
}


uint16_t touch_cb(pixel_t *pixel, uint16_t x, uint16_t y, void * param1, void * param2, void *param3)
{
    raster_t *raster;
    uint16_t *direction;
    pos_t *pos;
    raster = (raster_t *)param1;
    direction = (dir_t *)param2;
    pos = (pos_t*)param3;

    // check bounding box
    if(pos->x + x <= 0)
    {
        *direction |= left;
    }
    else if(fb_get_pixel(raster,pos->x + x - 1, pos->y + y)->flags & R_VISIBLE)
    {
        fb_get_pixel(raster,pos->x + x - 1, pos->y + y)->flags |= R_TOUCH;
        *direction |= left;
    }

    if(pos->x + x >= raster->x_max - 1)
    {
        *direction |= right;
    }
    else if(fb_get_pixel(raster,pos->x + x + 1, pos->y + y)->flags & R_VISIBLE)
    {
        fb_get_pixel(raster,pos->x + x + 1, pos->y + y)->flags |= R_TOUCH;
        *direction |= right;
    }

    if(pos->y + y <= 0)
    {
        *direction |= up;
    }
    else if(fb_get_pixel(raster,pos->x + x, pos->y + y - 1)->flags & R_VISIBLE)
    {
        fb_get_pixel(raster,pos->x + x, pos->y + y - 1)->flags |= R_TOUCH;
        *direction |= up;
    }

    if(pos->y + y >= raster->y_max - 1)
    {
        *direction |= down;
    }
    else if(fb_get_pixel(raster,pos->x + x, pos->y + y + 1)->flags & R_VISIBLE)
    {
        fb_get_pixel(raster,pos->x + x, pos->y + y + 1)->flags |= R_TOUCH;
        *direction |= down;
    }
    return 0;

}

/**
 * Returns which edge a sprite has made contact
 */
uint16_t sprite_touching(raster_t *raster, raster_t *sprite, pos_t pos)
{
    uint16_t touch = 0;
    sprite_parser(sprite,R_VISIBLE, touch_cb, raster, &touch, &pos);
    return touch;
}

struct check_rot_s{
    pos_t        position;  // Position of sprite within raster
    pos_t        origin;    // point of rotation * 2
    transform_t  rotation;  // rotation type
};

uint16_t rotated_cb(pixel_t *pixel, uint16_t x, uint16_t y, void * param1, void * param2, void *param3)
{
    uint16_t *result;
    raster_t *raster;
    struct check_rot_s *details;
    uint16_t touching;
    details = (struct check_rot_s *)param2;
    raster = (raster_t *)param1;
    result = (uint16_t *)param3;

    pos_t normal_pos = {x,y};

    // apply rotation
    normal_pos = pos_rotate(normal_pos, details->origin, details->rotation);
    normal_pos.x += details->position.x;
    normal_pos.y += details->position.y;

    touching = pos_out_raster(raster, normal_pos);
    if(touching)
    {
        *result |= touching;
    }
    else
    {
        // get pixel in raster at this rotated location
        if(fb_get_pixel(raster, normal_pos.x, normal_pos.y)->flags & R_VISIBLE )
        {
            *result |= 1;
        }
        fb_get_pixel(raster, normal_pos.x, normal_pos.y)->flags |= R_PARTIAL;
    }
    return 1;
}

/**
 * returns true if sprite can be rotated
 */
uint16_t sprite_can_rotate(raster_t *raster, raster_t *sprite, pos_t position, transform_t rotate)
{
    uint16_t result = 0;
    struct check_rot_s details;
    details.position = position;
    details.origin = (pos_t){sprite->x_max-1, sprite->y_max-1};
    details.rotation = rotate;
    sprite_parser(sprite,R_VISIBLE, rotated_cb, raster, &details, &result);

    return result == 0;
}

/**
 * Rotate a square raster
 */
uint16_t sprite_transform(raster_t *sprite, transform_t rotate)
{
    uint16_t x;
    uint16_t y;
    pixel_t temp_pixel;
    if(sprite->x_max != sprite->y_max)
    {
        return 0;
    }
    if(rotate == tr_rot_left)
    {
        for(y=0; y<(sprite->y_max/2); y++)
        {
            for(x=y; x<sprite->x_max-y-1; x++)
            {
                temp_pixel = *fb_get_pixel(sprite,x, y);
                *fb_get_pixel(sprite,x, y) = *fb_get_pixel(sprite,sprite->x_max-1-y, x);
                *fb_get_pixel(sprite,sprite->x_max-1-y, x) = *fb_get_pixel(sprite,sprite->x_max-1-x, sprite->y_max-1-y);
                *fb_get_pixel(sprite,sprite->x_max-1-x, sprite->y_max-1-y) = *fb_get_pixel(sprite,y, sprite->y_max-1-x);
                *fb_get_pixel(sprite,y, sprite->y_max-1-x) = temp_pixel;
            }
        }
    }
    else if(rotate == tr_rot_right)
    {
        for(y=0; y<(sprite->y_max/2); y++)
        {
            for(x=y; x<sprite->x_max-y-1; x++)
            {
                temp_pixel = *fb_get_pixel(sprite,x, y);
                *fb_get_pixel(sprite,x, y) = *fb_get_pixel(sprite,y, sprite->y_max-1-x);
                *fb_get_pixel(sprite,y, sprite->y_max-1-x) = *fb_get_pixel(sprite,sprite->x_max-1-x, sprite->y_max-1-y);
                *fb_get_pixel(sprite,sprite->x_max-1-x, sprite->y_max-1-y) = *fb_get_pixel(sprite,sprite->x_max-1-y, x);
                *fb_get_pixel(sprite,sprite->x_max-1-y, x) = temp_pixel;
            }
        }
    }
    return 1;
}


