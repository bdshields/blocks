/*
 * debug.c
 *
 *  Created on: 14 Sep 2020
 *      Author: brandon
 */


#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

#include "debug.h"


dbg_type    dbg_enabled = 0;
dbg_verbose dbg_level = dbg_none;

void dbg_enable(dbg_type mask)
{
    dbg_enabled |= mask;
}

void dbg_disable(dbg_type mask)
{
    dbg_enabled &= ~mask;
}

void dbg_setLevel(dbg_verbose level)
{
    dbg_level = level;
}

void dbg_dprintf(char * file, uint16_t line, char * format, ...)
{
    va_list ap;
    va_start (ap, format);
    fprintf(stderr, "%s: %d: ",file, line);
    vfprintf(stderr, format, ap);
    va_end (ap);
}
