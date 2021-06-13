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
#include "debug.h"

#define DEBUG_LEVEL DBG_NONE

// The Fastest Fourier Transform in the West
#include <complex.h>
#include <fftw3.h>
#include <math.h>

#include <stdlib.h>


#define _1_DB (1.25)

const raster_t specgram_logo = {
    .x_max = 5,
    .y_max = 5,
    .image = {PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,
              PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_RED__,PX_CLEAR,
              PX_CLEAR,PX_RED__,PX_CLEAR,PX_RED__,PX_CLEAR,
              PX_RED__,PX_RED__,PX_RED__,PX_RED__,PX_RED__,
              PX_RED__,PX_RED__,PX_RED__,PX_RED__,PX_RED__,

    }
};

raster_t *specgram_option(void)
{
    return &specgram_logo;
}


#define AUDIO_FRAMES (12000)  // Frames grabbed from ALSA each round
#define AUDIO_FFT_MULTI (4)  // Multiplier of AUDIO_FRAMES for FFT
#define AUDIO_FFT (AUDIO_FRAMES * AUDIO_FFT_MULTI)
#define AUDIO_SAMPLERATE (ALSA_SRATE)

#define FREQ_STEP ((float)(AUDIO_SAMPLERATE/2.0)/((float)(AUDIO_FFT)))

int32_t specgram_freqs []=
{
        25,
        40,
        63,
        100,
        160,
        250,
        400,
        630,
        1000,
        1600,
        2500,
        4000,
        6300,
        10000,
        16000
};

uint16_t specgram_calibration[]=
{
        8, 10, 10, 10, 10, 12, 13, 13, 13, 13, 13, 12, 12, 14, 2
};

// strength to vertical increment mapping
#define _map_5(_a) _a, _a, _a, _a, _a
#define _map_4(_a) _a, _a, _a, _a
#define _map_3(_a) _a, _a, _a
#define _map_2(_a) _a, _a
#define _map_1(_a) _a

uint16_t db_scaling2[]=
{
        _map_1(0),
        _map_1(1),
        _map_1(2),
        _map_1(3),
        _map_1(4),
        _map_1(5),
        _map_1(6),
        _map_1(7),
        _map_1(8),
        _map_1(9),
        _map_1(10),
        _map_1(11),
        _map_1(12),
        _map_1(13),
        _map_1(14),
};

pixel_t spec_colours2[][3]={
        // Column                       top                    peak
        {{255, 0, 230, R_VISIBLE},{255, 148, 239, R_VISIBLE},{255, 102, 240, R_VISIBLE}},
        {PX_PURPL,                {255, 148, 239, R_VISIBLE},{255, 200, 30, R_VISIBLE}}
};

double mel_scale2(double *input, int32_t length, int32_t center);
void abs_array(double *data, int32_t length);
pixel_t colourize_spec(int16_t intensity);

struct _peak_s{
    uint16_t value;
    int16_t  delay;
};

typedef int16_t sound_frame[AUDIO_FRAMES][2];

void specgram_run(uint16_t x, uint16_t y)
{
    raster_t        *screen;
    user_input_t    button;
    snd_pcm_t       *sndHandle;
    uint16_t        counter;

    //int16_t         sounddata[AUDIO_FFT_MULTI][AUDIO_FRAMES][2];
    sound_frame    *sounddata;
    uint16_t        fft_sequence = 0;  // variable to round robin through AUDIO_FFT_MULTI
    uint16_t        fft_count;
    uint16_t        in_counter;
    uint16_t        out_counter;

    uint16_t        spec_data[y][x];
    uint16_t        spec_index=0;

    int32_t         frame_min = 1000000;
    int32_t         frame_max = -1000000;

    int32_t         min_value = 10;
    int32_t         max_value = 50;

    DEBUG("Starting\n");
    sounddata = malloc(sizeof(sound_frame)*AUDIO_FFT_MULTI);
    if (sounddata == NULL)
    {
        DEBUG("Failed to allocate buffer\n");
        exit(1);
    }

    // Data types from the fastest fourier transform in the west
    double *in, *out;
    fftw_plan p;

    in = (double*) fftw_malloc(sizeof(double) * AUDIO_FFT);
    out = (double*) fftw_malloc(sizeof(double) * AUDIO_FFT);

    if((in == NULL) || (out == NULL))
    {
        DEBUG("Failed to allocate FFT buffer\n");
        exit (1);
    }

    DEBUG("Preparing\n");

    p = fftw_plan_r2r_1d(AUDIO_FFT, in, out, FFTW_REDFT10, FFTW_MEASURE);

    DEBUG("Openning\n");
    sndHandle = alsa_open(DEVICE);
    screen = fb_allocate(x, y);


    while(1)
    {
        alsa_read(sndHandle, sounddata[fft_sequence], AUDIO_FRAMES);

        // Increment
        fft_sequence = (fft_sequence + 1) % AUDIO_FFT_MULTI;

        out_counter=0;
        for(fft_count=0; fft_count<AUDIO_FFT_MULTI; fft_count++)
        {
            // copy data into fft input
            for(in_counter=0; in_counter<AUDIO_FRAMES; in_counter++)
            {
                in[out_counter++] = sounddata[fft_sequence][in_counter][0];
            }
            fft_sequence = (fft_sequence + 1) % AUDIO_FFT_MULTI;
        }

        // Execute the DCT
        fftw_execute(p);

        abs_array(out, AUDIO_FFT/2);

        frame_min =  1000000;
        frame_max = -1000000;
        for (counter=1; counter<14; counter++)
        {
            int16_t     level;
            float       value;

            level = 0;

            value = mel_scale2(out, AUDIO_FFT/2, specgram_freqs[counter]/FREQ_STEP);

            // Log scale
            while(value > 1)
            {
                level ++;
                value /= _1_DB ;
            }

            // apply calibration
           // level -= specgram_calibration[counter];

            // gather statistics
            frame_min = level < frame_min ? level : frame_min;
            frame_max = level > frame_max ? level : frame_max;

            // restrict to range
            if(level < min_value)
            {
                level = min_value;
            }
            if(level > max_value)
            {
                level = max_value;
            }

            spec_data[14 - counter][spec_index] = level - min_value;

        }
        // Fix some dodg
        spec_data[0][spec_index] = spec_data[1][spec_index];
        spec_data[14][spec_index] = spec_data[13][spec_index];



        spec_index ++;
        if (spec_index >= x)
        {
            spec_index=0;
        }
        fb_clear(screen);
        for(counter = 0; counter < (x*y); counter++)
        {
            screen->image[counter] = colourize_spec(((uint16_t*)spec_data)[counter] * (8192 / (max_value - min_value)));
        }
        frame_drv_render(screen);


        // update long running stats
        min_value += (frame_min - min_value)>>3;
        max_value += (frame_max - max_value)>>3;
        if((max_value - min_value) < 20)
        {
            max_value = min_value + 20;
        }

        DEBUG("Min_value = (%d)%d, Max_value = (%d)%d, frame %d\n",frame_min,min_value, frame_max,max_value, (int32_t)spec_index);


        button = in_get_bu();
        switch(button.button)
        {
        case bu_start:
            goto exit;
        }
    }

exit:
    fftw_destroy_plan(p);
    fftw_free(in);
    fftw_free(out);
    free(sounddata);

    fb_destroy(screen);
    alsa_close(sndHandle);
}

/**
 * Calculate mel scaled value for the given center point
 *
 * @param input Array of input data
 * @param length Length of input data
 * @param center Calculate energy for value
 *
 */
double mel_scale2(double *input, int32_t length, int32_t center)
{
    int32_t counter;
    float    window_start;
    float    window_end;
    double    result;

    double tan_left;
    double tan_right;

    result = 0;

    window_start = center / 1.7;
    window_end = center * 1.7;

    tan_left = (1.0/((float)center-window_start));
    tan_right = (1.0/(window_end - (float)center));

    // Skip the first entry, it's usually DC offset.
    for (counter =1; counter<length; counter++)
    {
        if(counter < window_start)
        {
            continue;
        }
        else if(counter > window_end)
        {
            break;
        }
        else if(counter < center)
        {
            result += tan_left * ((double)counter - window_start) * input[counter];
        }
        else if(counter == center)
        {
            result += input[counter];
        }
        else if(counter > center)
        {
            result += tan_right * (window_end - (double)counter) * input[counter];
        }
    }
    return result;
}

pixel_t colourize_spec(int16_t intensity)
{
    int32_t value;
    pixel_t colours;
    int16_t red=0;
    int16_t green=0;
    int16_t blue=0;


    value = intensity;

    switch (2)
    {
    case 0:

        value >>= 7;  // 0 -> 512


        blue = value - 256 + 64;
        if(blue > 255)
        {
            blue = 255;
        }
        else if(blue < 0)
        {
            blue = 0;
        }
        green = -(value - 255 - 64);
        if(green > 255)
        {
            green = 255;
        }
        else if(green < 0)
        {
            green = 0;
        }

        break;
    case 1:

        value >>= 7;  // 0 - 512


        red = value - 256 + 64;
        if(red > 255)
        {
            red = 255;
        }
        else if(red < 0)
        {
            red = 0;
        }
        blue = -(value - 255 - 64);
        if(blue > 255)
        {
            blue = 255;
        }
        else if(blue < 0)
        {
            blue = 0;
        }

        break;
    case 2:
        value >>= 3;  // 0 -> 1024

        red = value - (256 * 3);
        if(red > 255)
        {
            red = 255;
        }
        else if(red < 0)
        {
            red = 0;
        }

        blue = value;
        if(blue > 255)
        {
            blue = 255;
        }
        else if(blue < 0)
        {
            blue = 0;
        }
        green = (value - 255) / 2;
        if(green > 255)
        {
            green = 255;
        }
        else if(green < 0)
        {
            green = 0;
        }

        break;
    }
    colours.blue =  blue;
    colours.green = green;
    colours.red = red;
    colours.flags = R_VISIBLE;
    return colours;
}

