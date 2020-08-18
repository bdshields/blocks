/*
 * frame_drv.h
 *
 *  Created on: 12 May 2019
 *      Author: brandon
 */

#ifndef SRC_FRAME_DRV_H_
#define SRC_FRAME_DRV_H_

#include "frame_buffer.h"

typedef enum drv_type_e {
    dr_none,
    dr_term,
    dr_ws2811
}drv_type;

typedef enum disp_state_e {
    ds_none,
    ds_active,
    ds_sleep
}disp_state;

int frame_drv_init(int x, int y, drv_type type);
int frame_drv_render(raster_t * raster);
int frame_drv_shutdown();

void frame_drv_standby();
void frame_drv_wake();

#endif /* SRC_FRAME_DRV_H_ */
