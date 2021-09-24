#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#if defined(__WATCOMC__)

#ifdef __386__
#include <i86.h>
#endif

#define inportb(x) inp(x)
#define outportb(x, v) outp(x, v)
#define getvect(x) _dos_getvect(x)
#define setvect(x, h) _dos_setvect(x, h)
#define enable _enable
#define disable _disable
#define INTERRUPT __interrupt __far

#elif defined(__TURBOC__)

#define INTERRUPT interrupt

#else

#error "No supported platform detected"

#endif

#endif