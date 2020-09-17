/*
 * image_util.h
 *
 *  Created on: 28 Apr. 2019
 *      Author: brandon
 */

#ifndef SRC_IMAGE_UTIL_H_
#define SRC_IMAGE_UTIL_H_


#include <stdint.h>
#include "frame_buffer.h"
#include "pos.h"




/**
 * callback function used by parser
 * @param pixel
 */
typedef uint16_t(*sp_cb)(pixel_t *pixel, uint16_t x, uint16_t y, void * param1, void * param2, void * param3);


uint16_t paste_sprite(raster_t *frame_buffer, raster_t *sprite, pos_t pos);
void sprite_parser(raster_t *sprite, uint16_t flag_mask, sp_cb callback_f, void *param1, void *param2, void * param3);
uint16_t sprite_touching(raster_t *raster, raster_t *sprite, pos_t pos);
uint16_t sprite_transform(raster_t *sprite, transform_t rotate);
uint16_t sprite_can_rotate(raster_t *raster, raster_t *sprite, pos_t origin, transform_t rotate);
uint16_t pos_out_raster(raster_t *raster, pos_t pos);

#endif /* SRC_IMAGE_UTIL_H_ */
