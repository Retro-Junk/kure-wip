#ifndef _COMMON_H_
#define _COMMON_H_

#include "platform.h"

#define VERSION_USA

#define BE(x) (((x) >> 8) | ((x) << 8))
#define LE16(x) (x)

#define TODO(s) \
	{   \
		printf(s);  \
		PromptWait();   \
		for(;;) ;   \
	}

typedef struct rect_t {
	byte sx;
	byte ex;
	byte sy;
	byte ey;
} rect_t;

#if 0
#define DEBUG_ENDING
#endif

#if 0
#define DEBUG
#endif

#if 0
/*0 - play intro*/
/*0xFF - skip intro, quest item and teleport*/
/*other - skip intro, play quest item seq, teleport to room*/
#define DEBUG_SKIP_INTRO 0xFF
#endif

#if 1
#define DEBUG_ZONE
#endif

#if 0
/*Rope -> De Profundis*/
#define DEBUG_QUEST 0x00
#endif

#if 0
/*Knife -> The Wall*/
#define DEBUG_QUEST 0x40
#endif

#if 0
/*Goblet -> The Twins*/
#define DEBUG_QUEST 0x80
#endif

#if 0
/*Fly -> Scorpion's*/
#define DEBUG_QUEST 0xC0
#endif

#if 0
/*win in fights*/
#define CHEAT
#endif

#if 0
/*never lose to the Skull Trader*/
#define CHEAT_TRADER
#endif

#endif
