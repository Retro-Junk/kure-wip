#include "common.h"
#include "resdata.h"

#if defined(VERSION_US_EGA)
byte perso_data[RES_PERSO_MAX];
#else
byte pers1_data[RES_PERS1_MAX];
byte pers2_data[RES_PERS2_MAX];
#endif

byte desci_data[RES_DESCI_MAX];
byte diali_data[RES_DIALI_MAX];
