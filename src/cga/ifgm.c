#include <dos.h>
#include "common.h"
#include "ifgm.h"
#include "dialog.h"
#include "sound.h"

byte ifgm_loaded = 0;
byte ifgm_flag2;

void IFGM_Init(void) {
#ifdef VERSION_USA
	/*TODO*/
#else
	ifgm_loaded = 0;
#endif
}

void IFGM_Shutdown(void) {

}

void IFGM_Poll(void) {
	if(!ifgm_loaded)
		return;
/*
xor     ax, ax
int     0F0h
*/

}


/*
Load and play a sound sample.
Return 0 if playback is unavailable
*/
int IFGM_PlaySound(byte index) {
	if (!ifgm_loaded)
		return 0;

	/*TODO*/

	return 0;
}

void IFGM_PlaySample(byte index) {
	if (!ifgm_loaded)
		return;
	IFGM_PlaySound(index);
}

void IFGM_StopSample(void) {
	union REGS reg;
	if (!ifgm_loaded)
		return;
#ifdef __TURBOC__
	reg.h.ah = 0xC;
	reg.x.dx = 64;
	reg.x.si = 0;
	int86(0xF0, &reg, &reg);
#endif
}

static byte sfx_sounds[] = {
	0, 5, 10, 14, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void IFGM_PlaySfx(byte index) {
	if (!ifgm_loaded)
		return;
	if (cur_dlg_index == 0)
		return;
	PlaySound(sfx_sounds[index % 16]);
}
