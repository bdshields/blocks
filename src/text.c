/*
 * text.c
 *
 *  Created on: 11 Apr 2021
 *      Author: brandon
 */

#include "pos.h"
#include "image_util.h"
#include "frame_buffer.h"
#include <stddef.h>
#include "font.h"

void text_print(raster_t *canvas, pos_t position, pixel_t colour, char buffer[])
{
    char *character = buffer;
    pos_t out_pos = position;
    uint8_t x;
    uint8_t y;
    uint8_t font_width;
    uint8_t font_height;

    uint8_t *font;

    get_font_size(&font_width, &font_height);
    // For each character
    while (*character != NULL)
    {
        // Get the font bitmap
        font = get_char(*character);
        x=0;
        y=0;
        // for each bit, check if set and output
        while(y<font_height)
        {
            if(*font == 1)
            {
                if (0 == pos_out_raster(canvas, pos_add(out_pos,POS(x,y))))
                {
                    // set the active pixel to the desired colour
                    *fb_get_pixel(canvas, out_pos.x + x, out_pos.y + y) = colour;
                }
            }
            x++;
            if (x == font_width)
            {
                x = 0;
                y++;
            }
            font ++;
        }
        out_pos.x += font_width;
        character++;
    }
}

