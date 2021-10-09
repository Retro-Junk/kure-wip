#include "common.h"
#include "room.h"
#include "script.h"

#ifdef __TURBOC__

#if sizeof(rect_t) != 4
#error "rect_t must be 4 bytes long"
#endif

#if sizeof(spot_t) != 8
#error "spot_t must be 8 bytes long"
#endif

#if sizeof(pers_t) != 5
#error "pers_t must be 5 bytes long"
#endif

#if sizeof(item_t) != 6
#error "item_t must be 6 bytes long"
#endif

#else

char byte_size_check[1 - 2*(sizeof(byte) != 1)];
char short_size_check[1 - 2*(sizeof(int16) != 2)];
char rect_size_check[1 - 2*(sizeof(rect_t) != 4)];
char spot_size_check[1 - 2*(sizeof(spot_t) != 8)];
char pers_size_check[1 - 2*(sizeof(pers_t) != 5)];
char item_size_check[1 - 2*(sizeof(item_t) != 6)];

#endif
