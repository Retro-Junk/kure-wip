#ifndef _CURSOR_H_
#define _CURSOR_H_

#define CURSOR_WIDTH 16
#define CURSOR_HEIGHT 16
#define CURSOR_WIDTH_SPR 20

enum Cursors {
	CURSOR_FINGER,
	CURSOR_TARGET,
	CURSOR_FLY,
	CURSOR_SNAKE,
	CURSOR_GRAB,
	CURSOR_POUR,
	CURSOR_BODY,
	CURSOR_ARROWS,
	CURSOR_CROSSHAIR,
	CURSOR_MAX
};

extern unsigned int cursor_x;
extern unsigned char cursor_y;
extern unsigned char cursor_color;
extern unsigned char *cursor_shape;
extern unsigned char cursor_anim_ticks;
extern unsigned char cursor_anim_phase;

void SelectCursor(unsigned int num);
void UpdateCursor(void);
void DrawCursor(unsigned char *target);
void UndrawCursor(unsigned char *target);
void UpdateUndrawCursor(unsigned char *target);

#endif
