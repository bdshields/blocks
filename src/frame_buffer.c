#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "frame_buffer.h"


raster_t *fb_allocate(uint16_t x, uint16_t y)
{
    raster_t *raster;

    raster = malloc(sizeof(raster_t) + sizeof(pixel_t)*x*y);
    if(raster)
    {
        raster->x_max = x;
        raster->y_max = y;
        raster->center = POS(0,0);
    }
    return raster;
}

raster_t *fb_copy(raster_t * source)
{
    raster_t *raster;
    uint16_t sizeofRaster;

    sizeofRaster = sizeof(raster_t) + sizeof(pixel_t)*source->x_max*source->y_max;

    raster = malloc(sizeofRaster);
    if(raster)
    {
        memcpy(raster, source, sizeofRaster);
    }
    return raster;
}

void fb_clear(raster_t *frame_buffer)
{
    memset(frame_buffer->image, 0, IMAGE_SIZE(frame_buffer->x_max, frame_buffer->y_max));
}


void fb_destroy(raster_t *raster)
{
    if(raster)
    {
        free(raster);
    }
}
