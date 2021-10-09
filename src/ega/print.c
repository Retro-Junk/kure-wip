#include "common.h"
#include "cga.h"
#include "ega.h"
#include "cursor.h"
#include "dialog.h"

byte *cur_str_end;

byte draw_x;
byte draw_y;

byte char_draw_coords_x;
byte char_draw_coords_y;
byte string_ended;
byte char_draw_max_width;
byte char_draw_max_height;

/*
Calculate number of string's character until whitespace
Return current word's characters count and the next word ptr
*/
byte *CalcStringWordWidth(byte *str, unsigned int *w) {
	unsigned int ww = 0;
	byte c;

	if ((*str & 0x3F) == 0) {
		str++;
		ww++;
	}

	while (str != cur_str_end) {
		if ((*str & 0x3F) == 0)
			break;
		ww++;
		c = *str & 0xC0;
		if (c != 0) {
			if (c == 0x40) { /*space?*/
				str++;
				break;
			}
			ww++;
		}
		str++;
	}

	*w = ww;
	return str;
}

/*
Calculate number of text's words and max word width (in chars)
*/
void CalcStringSize(byte *str, unsigned int *w, unsigned int *n) {
	unsigned int ww = 0, nw = 0, lw;
	byte *s = str;
	do {
		s = CalcStringWordWidth(s, &lw);
		if (lw > ww)
			ww = lw;
		nw += 1;
	} while (s != cur_str_end);
	*w = ww;
	*n = nw;
}

/*
Calculate number of text's lines with respect to set max width
If a line is longer, wrap it to the next line
*/
unsigned int CalcTextLines(byte *str) {
	unsigned int lines = 1;
	unsigned int w, left = char_draw_max_width;
	while (str != cur_str_end) {
		str = CalcStringWordWidth(str, &w);
		if (left > w) {
			left = left - w - 1;
		} else {
			lines++;
			left = char_draw_max_width - w - 1;
		}
	}
	return lines;
}

void PrintStringPad(unsigned int w, byte *target) {
	while (w--)
		EGA_PrintChar(0, target);
}

byte *PrintWord(byte *str, byte *target) {
	byte c, f;
	if ((*str & 0x3F) == 0)
		goto skip_1st;
	while (str != cur_str_end) {
		f = *str;
		c = f & 0x3F;
		if (c == 0) {
			if ((f & 0xC0) == 0)
				str++;
			return str;
		}
		EGA_PrintChar(c, target);

skip_1st:
		;
		f = *str & 0xC0;
		str++;
		if (f) {
			if (f == 0x80)
				EGA_PrintChar(0x25, target);
			else if (f != 0x40)
				EGA_PrintChar(0x21, target);
			else
				return str;
		}
	}
	string_ended = 1;
	return str;
}

byte *PrintStringLine(byte *str, unsigned int *left, byte *target) {
	unsigned int mw = char_draw_max_width;
	for (;;) {
		unsigned int w;
		CalcStringWordWidth(str, &w);
		if (mw < w)
			break;
		mw -= w;
		str = PrintWord(str, target);
		if (string_ended || (mw == 0))
			break;
		mw--;
		EGA_PrintChar(0, target);
	}
	*left = mw;
	return str;
}

byte *PrintStringPadded(byte *str, byte *target) {
	unsigned int w;
	str = PrintStringLine(str, &w, target);
	if (w != 0)
		PrintStringPad(w, target);
	return str;
}

void PrintStringCentered(byte *str, byte *target) {
	byte pad = 0;
	unsigned int ww = 0, lw;
	byte *s = str;
	do {
		s = CalcStringWordWidth(s, &lw);
		ww += lw;
	} while (s != cur_str_end);

	pad = (char_draw_max_width - ww) / 2;
	if (pad) {
		char_draw_max_width -= pad;
		PrintStringPad(pad, target);
	}
	string_ended = 0;   /*TODO: move me elsewhere*/
	PrintStringPadded(str, target);
}

void DrawTextBox(byte *msg, byte *target) {
	unsigned int x, y, w, i;
	unsigned int ww, nw;

	EGA_SetDrawColor(0x0E00);

	CalcStringSize(msg, &ww, &nw);
	if (ww >= char_draw_max_width)
		char_draw_max_width = ww;

	x = draw_x * 4;
	y = draw_y;
	w = (char_draw_max_width + 2) * 4 - 2;

	EGA_DrawHLine(x + 2, y, w - 2, 0, target);          /*box top*/
	for (i = 0; i < 3; i++)
		EGA_DrawHLine(x + 1, y + 1 + i, w, 14, target);	/*top margin*/
	EGA_DrawVLine(x,     y + 2, 2, 0, target);			/*left top corner 1*/
	EGA_DrawVLine(x + 1, y + 1, 1, 0, target);			/*left top corner 2*/
	EGA_DrawVLine(x + w, y + 1, 1, 0, target);			/*right top corner 1*/
	EGA_DrawVLine(x + w + 1, y + 2, 2, 0, target);		/*right top corner 2*/

	char_draw_coords_y = draw_y + 4;
	string_ended = 0;
	do {
		char_draw_coords_x = draw_x;
		EGA_PrintChar(0x3B, target);
		msg = PrintStringPadded(msg, target);
		EGA_PrintChar(0x3C, target);
		char_draw_coords_y += 6;
	} while (!string_ended);

	x = draw_x * 4;
	y = char_draw_coords_y;
	EGA_DrawHLine(x + 1, y, w, 14, target);				/*bottom margin*/
	EGA_DrawVLine(x + 1, y, 1, 0, target);				/*bottom left corner 1*/
	EGA_DrawHLine(x + 2, y + 1, w - 2, 0, target);		/*box bottom*/
	EGA_DrawVLine(x + 1, y, 1, 0, target);				/*TODO: duplicated?*/
	EGA_DrawVLine(x + w, y, 1, 0, target);				/*bottom right corner*/
}

void DrawMessage(byte *msg, byte *target) {
	unsigned int x, y;
	unsigned int w, h;
	CalcStringSize(msg, &w, &h);
	char_draw_max_width = (h < 5) ? (w + 2) : 20;
	char_draw_max_height = CalcTextLines(msg) * 6 + 7;

	x = cursor_x / 4;
	if (x < 9)
		x = 9;
	if (x + char_draw_max_width + 2 >= 73)
		x = 73 - (char_draw_max_width + 2);

	y = cursor_y;
	if (y + char_draw_max_height >= 200)
		y = 200 - char_draw_max_height;

	draw_x = x;
	draw_y = y;

	CGA_BackupImageReal(CGA_CalcXY_p(x, y), char_draw_max_width + 2, char_draw_max_height); /*backup orig. screen data*/
	DrawTextBox(msg, target);					/*draw box with text*/
	PromptWait();								/*wait keypress*/
	CGA_RestoreBackupImage(target);				/*restore screen data*/
}
