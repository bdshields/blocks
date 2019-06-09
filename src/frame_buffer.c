#include <stdlib.h>
#include <stdio.h>

#include "frame_buffer.h"


raster_t *fb_allocate(uint16_t x, uint16_t y)
{
    raster_t *raster;

    raster = malloc(sizeof(raster_t) + sizeof(pixel_t)*x*y);
    if(raster)
    {
        raster->x_max = x;
        raster->y_max = y;
    }
    return raster;
}


void fb_destroy(raster_t *raster)
{
    if(raster)
    {
        free(raster);
    }
}
