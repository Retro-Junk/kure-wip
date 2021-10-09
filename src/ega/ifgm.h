#ifndef _IFGM_H_
#define _IFGM_H_

extern byte ifgm_loaded;

extern uint16 ifgm_flag1;
extern byte ifgm_flag2;

void IFGM_Init(void);

void IFGM_Shutdown(void);

void IFGM_Poll(void);

void IFGM_PlaySample(byte index);
void IFGM_StopSample(void);

int IFGM_PlaySound(byte index);
void IFGM_PlaySfx(byte index);

#endif
