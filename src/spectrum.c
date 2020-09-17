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

// The Fastest Fourier Transform in the West
#include <complex.h>
#include <fftw3.h>
#include <math.h>


#define NOISE_DB 55

#define PEAK_DECAY_RATE  1
#define PEAK_DECAY_DELAY 9

#define _1_DB (1.25)

const raster_t spec_logo = {
    .x_max = 5,
    .y_max = 5,
    .image = {PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_CLEAR,
              PX_CLEAR,PX_CLEAR,PX_CLEAR,PX_RED__,PX_CLEAR,
              PX_CLEAR,PX_RED__,PX_CLEAR,PX_RED__,PX_CLEAR,
              PX_RED__,PX_RED__,PX_RED__,PX_RED__,PX_RED__,
              PX_RED__,PX_RED__,PX_RED__,PX_RED__,PX_RED__,

    }
};

raster_t *spec_option(void)
{
    return &spec_logo;
}


#define AUDIO_FRAMES (2048)  // Frames grabbed from ALSA each round
#define AUDIO_FFT_MULTI (2)  // Multiplier of AUDIO_FRAMES for FFT
#define AUDIO_FFT (AUDIO_FRAMES * AUDIO_FFT_MULTI)
#define AUDIO_SAMPLERATE (ALSA_SRATE)

#define FREQ_STEP ((float)(AUDIO_SAMPLERATE/2.0)/((float)(AUDIO_FFT)))

uint16_t spec_freqs []=
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

uint16_t spec_calibration[]=
{
        8, 10, 10, 10, 10, 12, 13, 13, 13, 13, 13, 12, 12, 14, 2
};

// strength to vertical increment mapping
#define _map_5(_a) _a, _a, _a, _a, _a
#define _map_4(_a) _a, _a, _a, _a
#define _map_3(_a) _a, _a, _a
#define _map_2(_a) _a, _a
#define _map_1(_a) _a

uint16_t db_scaling[]=
{
        _map_5(0),
        _map_2(1),
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
        _map_2(12),
        _map_2(13),
        _map_5(14),
};

pixel_t spec_colours[][3]={
        // Column                       top                    peak
        {{255, 0, 230, R_VISIBLE},{255, 148, 239, R_VISIBLE},{255, 102, 240, R_VISIBLE}},
        {PX_PURPL,                {255, 148, 239, R_VISIBLE},{255, 200, 30, R_VISIBLE}}
};

double mel_scale(double *input, int16_t length, int16_t center);
void abs_array(double *data, uint16_t length);

struct _peak_s{
    uint16_t value;
    int16_t  delay;
};

void spec_run(uint16_t x, uint16_t y)
{
    raster_t        *screen;
    user_input_t    button;
    snd_pcm_t       *sndHandle;
    uint16_t        counter;
    struct _peak_s  spec_peaks[y];
    int16_t         sounddata[AUDIO_FFT_MULTI][AUDIO_FRAMES][2];
    uint16_t        fft_sequence = 0;  // variable to round robin through AUDIO_FFT_MULTI
    uint16_t        fft_count;
    uint16_t        in_counter;
    uint16_t        out_counter;

    // Data types from the fastest fourier transform in the west
    double *in, *out;
    fftw_plan p;

    in = (double*) fftw_malloc(sizeof(double) * AUDIO_FFT);
    out = (double*) fftw_malloc(sizeof(double) * AUDIO_FFT);

    p = fftw_plan_r2r_1d(AUDIO_FFT, in, out, FFTW_REDFT10, FFTW_MEASURE);

    memset(spec_peaks,0,y * sizeof(struct _peak_s));

    sndHandle = alsa_open(DEVICE);
    screen = fb_allocate(x, y);


    while(1)
    {
       // update_tmr = set_alarm(1000/OSC_FPS);

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
        fb_clear(screen);

        abs_array(out, AUDIO_FFT/2);

        for (counter=0; counter<15; counter++)
        {
            int16_t     level;
            float       value;

            level = 0;

            value = mel_scale(out, AUDIO_FFT/2, spec_freqs[counter]/FREQ_STEP);

            // Log scale
            while(value > 1)
            {
                level ++;
                value /= _1_DB ;
            }

            // apply calibration
            level -= spec_calibration[counter];

            // Subtract baseline value
            if(level > NOISE_DB)
            {
                level -= NOISE_DB;
            }
            else
            {
                level = 0;
            }


            // limit to what can be displayed
            if(level > sizeof(db_scaling)/sizeof(db_scaling[0]))
            {
                level = sizeof(db_scaling)/sizeof(db_scaling[0]) - 1;
            }

            // dB per graphical division
            level = db_scaling[level];


            // Maintain peaks
            spec_peaks[counter].delay ++;
            if(spec_peaks[counter].delay >= PEAK_DECAY_RATE)
            {
                spec_peaks[counter].delay = 0;
            }
            if((spec_peaks[counter].delay == 0) && (spec_peaks[counter].value > 0))
            {
                spec_peaks[counter].value --;
            }
            if(spec_peaks[counter].value < level)
            {
                spec_peaks[counter].value = level;
                spec_peaks[counter].delay = -PEAK_DECAY_DELAY;
            }

            // Draw Bar
            while(level >= 0)
            {
                *fb_get_pixel(screen, (counter*2), y - 1 -level) = spec_colours[1][0];
                *fb_get_pixel(screen, (counter*2)+1, y - 1 -level) = spec_colours[1][0];
                level --;
            }
            //Draw peaks
            *fb_get_pixel(screen, (counter*2), y - 1 -spec_peaks[counter].value) = spec_colours[1][2];
            *fb_get_pixel(screen, (counter*2) + 1, y - 1 -spec_peaks[counter].value) = spec_colours[1][2];
        }
        frame_drv_render(screen);


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
double mel_scale(double *input, int16_t length, int16_t center)
{
    uint16_t counter;
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

void abs_array(double *data, uint16_t length)
{
    uint16_t counter;
    for(counter = 0; counter< length; counter++)
    {
        if(*data < 0)
        {
            *data = *data * (-1);
        }
        data++;
    }
}
