/*
 * oscilloscope.c
 *
 *  Created on: 29 August 2020
 *      Author: brandon
 */

#include <string.h>
#include "frame_drv.h"
#include "frame_buffer.h"
#include "colours.h"
#include "button.h"
#include "utils.h"
#include "image_util.h"
#include "frame_buffer.h"
#include "alsa_tools.h"


#define OSC_FPS 30

const raster_t osci_logo = {
    .x_max = 5,
    .y_max = 5,
    .image = {PX_CLEAR,PX_PURPL,PX_CLEAR,PX_CLEAR,PX_CLEAR,
              PX_PURPL,PX_CLEAR,PX_PURPL,PX_CLEAR,PX_PURPL,
              PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_PURPL,PX_CLEAR,
              PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,
              PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,

    }
};


const pixel_t oscColours[][2]=
{
        {PX_GREEN, PX_BLUE_},
        {PX_PURPL, (pixel_t){128, 0, 255,R_VISIBLE}},
        {PX_RED__, PX_YELLO},
        {PX_WATERBLUE, PX_LITEBLUE},
        {PX_RED__, PX_PURPL},
};
#define OSC_NUM_PALETTES (sizeof(oscColours)/(2*sizeof(pixel_t)))

const pixel_t oscLine_end=PX_BLANK;

raster_t *osci_option(void)
{
    return &osci_logo;
}

#define AUDIO_OVERSAMPLE 20 // Samples per pixel
#define OSC_HISTORY 30  // historic data for effects
#define AUDIO_FRAMES (x * AUDIO_OVERSAMPLE)

#define OSC_FPS  30

void osci_run(uint16_t x, uint16_t y)
{
    raster_t        *screen;
    user_input_t    button;
    snd_pcm_t       *sndHandle;
    systime         update_tmr;
    uint16_t        x_idx;
    uint16_t        counter;
    uint16_t        subCounter;
    int32_t         sample;
    int32_t         pastSample;

    int16_t         sounddata[AUDIO_FRAMES][2];

    int16_t        osc_data[OSC_HISTORY][x];
    uint16_t        osc_hist_idx=0;
    uint16_t        hist_cntr;
    uint16_t        current_palette=0;

    sndHandle = alsa_open(DEVICE);
    screen = fb_allocate(x, y);
    memset(osc_data, 0, sizeof(osc_data));


    update_tmr = set_alarm(1000/OSC_FPS);
    while(1)
    {

        alsa_read(sndHandle, sounddata, AUDIO_FRAMES);

        if( alarm_expired(update_tmr))
        {
            update_tmr = set_alarm(1000/OSC_FPS);

            fb_clear(screen);
            x_idx = 0;

            osc_hist_idx++;
            osc_hist_idx %= OSC_HISTORY;
            // Process the audio data
            for(counter = 0; counter< AUDIO_FRAMES; counter+= AUDIO_OVERSAMPLE)
            {
                sample = 0;
                // Calc average
                for(subCounter = 0; subCounter<AUDIO_OVERSAMPLE;  subCounter++)
                {
                    sample += sounddata[counter + subCounter][0];
                }
                sample /= AUDIO_OVERSAMPLE;

                osc_data[osc_hist_idx][x_idx++] = sample;
            }

            // Update screen, draw oldest data first
            for(hist_cntr=0; hist_cntr<OSC_HISTORY; hist_cntr++)
            {
                pixel_t paint;

                osc_hist_idx++;
                osc_hist_idx %= OSC_HISTORY;
                if(hist_cntr == (OSC_HISTORY - 1))
                {
                    // Newest data
                    paint = oscColours[current_palette][0];
                }
                else{
                    // Old data
                    if(hist_cntr > OSC_HISTORY / 2)
                    {
                        float hist_fraction = (float)(hist_cntr - (OSC_HISTORY / 2)) / (float)(OSC_HISTORY / 2);
                        paint = pixel_blend(oscColours[current_palette][0],hist_fraction,oscColours[current_palette][1],1.0-hist_fraction);
                        paint = pixel_blend(paint,0.5,oscLine_end,0.5);
                    }
                    else
                    {
                        float hist_fraction = (float)hist_cntr / (float)(OSC_HISTORY / 2);
                        paint = pixel_blend(oscColours[current_palette][1],hist_fraction * 0.5,oscLine_end,(1.0-hist_fraction) * 0.5);
                    }
                }
                // For each value in history, scale and draw it
                pastSample = 0;
                for(x_idx=0; x_idx<x; x_idx++)
                {
                    // Scale value to fit within verticle limit of display
                    sample = osc_data[osc_hist_idx][x_idx] / (0x7FFF / (y / 2));

                    if(x_idx == 0)
                    {
                        fb_get_pixel(screen, x_idx, (y / 2) + sample)[0] = paint;
                    }
                    else
                    {
                        // draw line between last sample and this sample
                        do{
                            if(pastSample < sample)
                            {
                                pastSample++;
                            }
                            else if(pastSample > sample)
                            {
                                pastSample--;
                            }
                            fb_get_pixel(screen, x_idx, (y / 2) + pastSample)[0] = paint;
                        }
                        while(pastSample != sample);
                    }

                    pastSample = sample;

                }
            }

            frame_drv_render(screen);
        }

        button = in_get_bu();
        switch(button.button)
        {
        case bu_b:
            current_palette ++;
            current_palette %= OSC_NUM_PALETTES;
            break;
        case bu_start:
            goto exit;
        }
    }

exit:
    fb_destroy(screen);
    alsa_close(sndHandle);
}
