/*
 * main.c
 *
 *  Created on: 28 Apr. 2019
 *      Author: brandon
 */


#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>

#include <time.h> // for nano sleep

#include <string.h>
#include <signal.h>

#include <unistd.h>

#include "frame_buffer.h"
#include "image_util.h"
#include "pos.h"
#include "button.h"
#include "input.h"
#include "http_srv.h"
#include "sprites.h"
#include "frame_drv.h"
#include "utils.h"
#include "config_file.h"

#include "tetris.h"
#include "invaders.h"
#include "saver_ripples.h"
#include "pong.h"
#include "paint.h"
#include "oscilloscope.h"
#include "spectrum.h"
#include "conways.h"
#include "clock.h"


struct _game{
    void (*run)(uint16_t x, uint16_t y);
    raster_t *(*option)(void);
    int16_t min_players;
};

const struct _game games[]=
{
    {
        .run = tetris_run,
        .option = tetris_option,
        .min_players = 1,
    },
    {
        .run = invaders_run,
        .option = invaders_option,
        .min_players = 1,
    },
    {
        .run = pong_run,
        .option = pong_option,
        .min_players = 1,
    },
    {
        .run = paint_run,
        .option = paint_option,
        .min_players = 1,
    },
    {
        .run = conways_run,
        .option = conways_option,
        .min_players = 1,
    },
    {
        .run = osci_run,
        .option = osci_option,
        .min_players = 1,
    },
    {
        .run = spec_run,
        .option = spec_option,
        .min_players = 1,
    },
    {
        .run = ripples_run,
        .option = ripples_option,
        .min_players = 1,
    },
    {
        .run = clock_run,
        .option = clock_option,
        .min_players = 1,
    }
};

#define NUM_GAMES 9

#define SCR_WIDTH 30
#define SCR_HEIGHT 15

#define SCREEN_SAVER_TIME 1 // minutes

void clear_down();
void sig_handler(int sig);
void parseargs(int argc, char **argv);

drv_type option_display_type=dr_none;

uint8_t  terminalInput=0;

int main(int argc, char *argv[])
{
    uint16_t    menu_state=0;
    int16_t     counter;
    int16_t     selected;
    raster_t    *raster = NULL;
    raster_t    *options = NULL;
    raster_t    *selector;
    pos_t        pos_menu;
    pos_t        pos_selector;

    user_input_t    button;

    uint16_t    update_scr=0;

    systime     screenSaverTimeout;


    parseargs(argc, argv);

    signal (SIGTERM,sig_handler);
    signal (SIGINT,sig_handler);
    if(terminalInput)
    {
        signal (SIGHUP,sig_handler);
    }
    else
    {
        signal (SIGHUP,SIG_IGN);
    }

    srand(0);

    config_init(NULL);



    button_init();
    if(terminalInput)
    {
        in_init();
    }
    http_init();



    if(option_display_type == dr_none)
    {
        fprintf(stderr,"No display defined\n");
        goto end;
    }

    /**
     * allocate our main raster object
     */
    raster = fb_allocate(SCR_WIDTH, SCR_HEIGHT);
    options = fb_allocate(NUM_GAMES * 6, 5);
    selector = fb_allocate(NUM_GAMES * 6, 1);



    if(raster == NULL)
    {
        goto end;
    }

    frame_drv_init(SCR_WIDTH, SCR_HEIGHT, option_display_type);

    screenSaverTimeout = set_alarm(SCREEN_SAVER_TIME * 60 * 1000);
    counter = 0;
    selected = 0;
    while(1)
    {
        switch(menu_state)
        {
        case 0:
            // draw menu

            fb_clear(options);
            pos_selector = (pos_t){0,0};
            for(counter = 0; counter<NUM_GAMES; counter ++)
            {
                paste_sprite(options, games[counter].option(), pos_selector);
                pos_selector.x += 6;
            }
            menu_state = 1;
        case 1:
            // update selector
            screenSaverTimeout = set_alarm(SCREEN_SAVER_TIME * 60 * 1000);
            fb_clear(selector);
            paste_sprite(selector, &cursor, (pos_t){2+selected * 6, 0});
            update_scr = 1;
            menu_state = 2;
            break;
        case 2:
            button = in_get_bu();
            switch(button.button)
            {
            case bu_left:
                selected --;
                menu_state = 1;
                break;
            case bu_right:
                selected ++;
                menu_state = 1;
                break;
            case bu_up:
                selected -= 5;
                menu_state = 1;
                break;
            case bu_down:
                selected +=5;
                menu_state = 1;
                break;
            case bu_a:
            case bu_b:
                if((http_session_countActive() >= games[selected].min_players) || (button.user >= games[selected].min_players))
                {
                    games[selected].run(SCR_WIDTH, SCR_HEIGHT);
                    http_session_clrPlayers();
                    screenSaverTimeout = set_alarm(SCREEN_SAVER_TIME * 60 * 1000);
                    menu_state = 0;
                }
                break;
            case bu_none:
                frame_sleep(50);
                break;
            }

            if(selected < 0)
            {
                selected=0;
            }
            else if(selected>=NUM_GAMES)
            {
                selected = NUM_GAMES-1;
            }
        }
        if(alarm_expired(screenSaverTimeout))
        {
            cancel_alarm(&screenSaverTimeout);
            frame_drv_standby();
        }
        if(update_scr)
        {
            fb_clear(raster);
            paste_sprite(raster, options, (pos_t){0,2});
            paste_sprite(raster, selector, (pos_t){0,0});
            paste_sprite(raster, options, (pos_t){-30,10});
            paste_sprite(raster, selector, (pos_t){-30,8});
            frame_drv_render(raster);
            update_scr = 0;
        }

    }


end:
    if(raster)
    {
        fb_destroy(raster);
    }
    clear_down();
    exit(0);
}


void sig_handler(int sig)
{
    switch(sig)
    {
    case SIGTERM:   // Default kill signal
    case SIGINT:    // Control-C signal
    case SIGHUP:    // Hang-up
        fprintf(stderr,"Received signal: %s\n", strsignal(sig));
        clear_down();
        exit (0);
        break;
    }
}

void clear_down()
{
    frame_drv_shutdown();
    if(terminalInput)
    {
        in_close();
    }
    http_shutdown();
    frame_sleep(100);
    config_close();
}

void parseargs(int argc, char **argv)
{
    int c;
    while(1)
    {
        c = getopt (argc, argv, "hwti");
        if(c == -1)
        {
            break;
        }
        switch(c)
        {
        case 'h':
            fprintf(stderr,"%s <options>\n", argv[0]);
            fprintf(stderr,
                    "-h      - This help message\n"
#ifdef WS2811_LIB
                    "-w      - Display on LED matrix\n"
#else
                    "-w      - Display on LED matrix (DISABLED)\n"
#endif
                    "-t      - Display in Terminal\n"
                    "-i      - Enable terminal input (enabled with -t)\n"
                    );
            break;
        case 'w':
            option_display_type = dr_ws2811;
            break;
        case 't':
            option_display_type = dr_term;
            terminalInput = 1;
            break;
        case 'i':
            terminalInput = 1;
            break;
        }
    }
}
