/*
 * frame_buffer.h
 *
 *  Created on: 28 Apr. 2019
 *      Author: brandon
 */

#ifndef SRC_FRAME_BUFFER_H_
#define SRC_FRAME_BUFFER_H_

#include <stdint.h>

typedef struct pixel_s{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t flags;
} pixel_t;



/**
 * Define a raster time. can be used for sprites also.
 * x=y=0   This is Top Left corner
 *
 * x_max, y_max  This is BOTTOM Right
 */
typedef struct raster_s{
    uint16_t    x_max;
    uint16_t    y_max;
    uint16_t    flags;
    pixel_t     image[];
}raster_t;


/**
 * Common flags between pixel and raster
 */

#define R_VISIBLE   0x0001  // Object is visible
#define R_TOUCH     0x0002  // Object is touching another object
#define R_PARTIAL   0x0004  //

#define IMAGE_SIZE(_x, _y) ((_x)*(_y)*sizeof(pixel_t))

/**
 * Initialise the
 */
void fb_init();


static inline pixel_t *fb_get_pixel(raster_t * raster, uint16_t x, uint16_t y)
{
    if((x < raster->x_max)&&(y < raster->y_max))
    {
        return raster->image + (y*raster->x_max) + x;
    }
    else
    {
        return raster->image;
    }
}


/**
 * Allocate frame buffer
 * @param x width of the raster
 * @param y height of the raster
 *
 * @return pointer to allocated raster
 */
raster_t *fb_allocate(uint16_t x, uint16_t y);
void fb_clear(raster_t *frame_buffer);
raster_t *fb_copy(raster_t * source);

void fb_destroy(raster_t* raster);


#endif /* SRC_FRAME_BUFFER_H_ */
