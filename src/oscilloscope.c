/*
 * oscilloscope.c
 *
 *  Created on: 29 August 2020
 *      Author: brandon
 */

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

const raster_t osci_line = {
    .x_max = 1,
    .y_max = 1,
    .image = {PX_GREEN}
};
const raster_t osci_dim = {
    .x_max = 1,
    .y_max = 1,
    .image = {PX_DIMGREEN}
};


raster_t *osci_option(void)
{
    return &osci_logo;
}

#define AUDIO_OVERSAMPLE 20 // Samples per pixel

#define AUDIO_FRAMES (x * AUDIO_OVERSAMPLE)

#define OSC_FPS  30

void osci_run(uint16_t x, uint16_t y)
{
    raster_t        *screen;
    user_input_t    button;
    snd_pcm_t       *sndHandle;
    systime         update_tmr;
    pos_t           plotPos;
    uint16_t        counter;
    uint16_t        subCounter;
    uint16_t        dropCounter;
    int32_t         sample;
    int32_t         pastSample;

    int16_t        sounddata[AUDIO_FRAMES][2];

    sndHandle = alsa_open(DEVICE);
    screen = fb_allocate(x, y);


    update_tmr = set_alarm(1000/OSC_FPS);
    while(1)
    {

        alsa_read(sndHandle, sounddata, AUDIO_FRAMES);

        if( alarm_expired(update_tmr))
        {
            update_tmr = set_alarm(1000/OSC_FPS);

            fb_clear(screen);
            plotPos = (pos_t){0,0};
            pastSample = 0;

            for(counter = 0; counter< AUDIO_FRAMES; counter+= AUDIO_OVERSAMPLE)
            {
                sample = 0;
                // Calc average
                for(subCounter = 0; subCounter<AUDIO_OVERSAMPLE;  subCounter++)
                {
                    sample += sounddata[counter + subCounter][0];
                }
                sample /= AUDIO_OVERSAMPLE;

                // Scale value to fix within display
                sample /= (0x7FFF / (y / 2));

                if(counter == 0)
                {
                    plotPos.y = (y / 2) + sample;
                    paste_sprite(screen, &osci_line, plotPos);
                }
                else
                {
                    do{
                        if(pastSample < sample)
                        {
                            pastSample++;
                            plotPos.y = (y / 2) + pastSample;
                            paste_sprite(screen, &osci_dim, plotPos);
                        }
                        else if(pastSample > sample)
                        {
                            pastSample--;
                            plotPos.y = (y / 2) + pastSample;
                            paste_sprite(screen, &osci_dim, plotPos);
                        }
                        else
                        {
                            plotPos.y = (y / 2) + pastSample;
                            paste_sprite(screen, &osci_line, plotPos);
                        }
                    }
                    while(pastSample != sample);
                }

                pastSample = sample;
                plotPos.x ++;
            }

            frame_drv_render(screen);
        }

        button = in_get_bu();
        switch(button.button)
        {
        case bu_start:
            goto exit;
        }
    }

exit:
    fb_destroy(screen);
    alsa_close(sndHandle);
}
