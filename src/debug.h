/*
 * debug.h
 *
 *  Created on: 14 Sep 2020
 *      Author: brandon
 */

#ifndef SRC_DEBUG_H_
#define SRC_DEBUG_H_

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>

#define DBG_HTTPSRV     0x0001
typedef uint16_t dbg_type;

typedef enum dbg_verbose_e{
    dbg_none,
    dbg_info,
    dbg_debug
}dbg_verbose;

#define DBG_NONE(_info, _debug)
#define DBG_INFO(_info, _debug) _info
#define DBG_DEBUG(_info, _debug) _info _debug

void dbg_enable(dbg_type mask);
void dbg_disable(dbg_type mask);
void dbg_setLevel(dbg_verbose level);
void dbg_dprintf(char * file, uint16_t line, char * format, ...);


#define INFO(...) DEBUG_LEVEL(,fprintf(stderr, __VA_ARGS__))
#define DEBUG(...) DEBUG_LEVEL(,dbg_dprintf(basename(__FILE__), __LINE__, __VA_ARGS__))

#endif /* SRC_DEBUG_H_ */
