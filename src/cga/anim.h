#ifndef _ANIM_H_
#define _ANIM_H_

void PlayAnim(byte index, byte x, byte y);
void CopyScreenBlockWithDotEffect(byte *source, byte x, byte y, byte width, byte height, byte *target);

extern byte dot_effect_step;
extern unsigned int dot_effect_delay;

#endif
