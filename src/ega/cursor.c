#include "common.h"
#include "cursor.h"
#include "resdata.h"
#include "cga.h"
#include "ega.h"

byte cursor_color = 0;

byte *cursor_shape;
byte cursor_anim_ticks;
byte cursor_anim_phase;

/*cursors hotspot offsets*/
unsigned int cursor_shifts[CURSOR_MAX][2] = {
	{ 0, 0 },
	{ 7, 7 },
	{ 7, 7 },
	{ 0, 0 },
	{ 7, 7 },
	{ 0, 15 },
	{ 7, 7 },
	{ 7, 7 },
	{ 7, 7 }
};

unsigned int cursor_x_shift;
byte cursor_y_shift;

unsigned int cursor_x;
byte cursor_y;

/*
Select cursor shape and its hotspot
*/
void SelectCursor(unsigned int num) {
	cursor_x_shift = cursor_shifts[num][0];
	cursor_y_shift = cursor_shifts[num][1];
	cursor_shape = souri_data + num * CURSOR_WIDTH * CURSOR_HEIGHT * 2 / EGA_PIXELS_PER_BYTE;
}

/*
Build cursor sprite for its current pixel-grained position
*/
void UpdateCursor(void) {
	byte *cursor, *sprite;
	byte cursor_bit_shift;
	byte bitofs;
	unsigned int x, y;
	x = cursor_x - cursor_x_shift;
	if ((signed int)x < 0) x = 0;
	y = cursor_y - cursor_y_shift;
	if ((signed int)y < 0) y = 0;

	cursor_bit_shift = x % 8;
	ega_cursor_params->drawoffs = EGA_CalcXY(x, y, &bitofs);

	cursor = cursor_shape;
	sprite = sprit_load_buffer;

	for (y = 0;y < CURSOR_HEIGHT; y++) {
		byte p0 = *cursor++;
		byte p1 = *cursor++;
		byte p2 = *cursor++;
		byte p3 = *cursor++;
		byte p4 = 0;
		byte p5 = 0;
		if (cursor_bit_shift != 0) {
			p4 = p0 << (7 - cursor_bit_shift);
			p5 = p2 << (7 - cursor_bit_shift);

			p0 = (p0 >> cursor_bit_shift) | (p1 << (7 - cursor_bit_shift));
			p1 = p1 >> cursor_bit_shift;

			p2 = (p2 >> cursor_bit_shift) | (p3 << (7 - cursor_bit_shift));
			p3 = p3 >> cursor_bit_shift;
		}

		sprite[0] = p1;
		sprite[1] = p3;
		sprite[2] = p0;
		sprite[3] = p2;
		sprite[4] = p4;
		sprite[5] = p5;

		sprite += 6;
	}
}

/*
Draw cursor sprite and backup background pixels
TODO: rename to ShowCursor
*/
void DrawCursor(void) {
	UpdateCursor();
	EGA_DrawCursor();
	EGA_Flip();
	UpdateCursor();
	EGA_DrawCursor();
}

/*
Restore background pixels under cursor
TODO: rename to HideCursor
*/
void UndrawCursor(void) {
	EGA_UndrawCursorBoth();
}

/*
Restore pixels under cursor and update cursor sprite
*/
void UpdateUndrawCursor(void) {
	/*TODO: does this call order makes any sense?*/
	UpdateCursor();
	UndrawCursor();
}
