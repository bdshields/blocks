/*
 * html.h
 *
 *  Created on: 6 May 2019
 *      Author: brandon
 */

#ifndef SRC_HTML_H_
#define SRC_HTML_H_

#define STR2(x) #x
#define STR(x) STR2(x)

#define INCHTML(name, file) \
    __asm__(".section .rodata\n" \
            ".global html_" STR(name) "_start\n" \
            ".balign 16\n" \
            "html_" STR(name) "_start:\n" \
            ".incbin \"" file "\"\n" \
            \
            ".global html_" STR(name) "_end\n" \
            ".balign 1\n" \
            "html_" STR(name) "_end:\n" \
            ".byte 0\n" \
    ); \
    extern const __attribute__((aligned(16))) void* html_ ## name ## _start; \
    extern const void* html_ ## name ## _end; \



#if 0
            ".type html_" STR(name) "_start, @object\n"
            ".type html_" STR(name) "_end, @object\n"
#endif

#endif /* SRC_HTML_H_ */
