#ifndef _PRINT_H_
#define _PRINT_H_

extern byte *cur_str_end;

extern byte draw_x;
extern byte draw_y;

extern byte chars_color_bonw[];
extern byte chars_color_bonc[];
extern byte chars_color_wonb[];
extern byte chars_color_wonc[];

void PrintStringCentered(byte *str, byte *target);
byte *PrintStringPadded(byte *str, byte *target);

void DrawMessage(byte *msg, byte *target);

void CGA_DrawTextBox(byte *msg, byte *target);

#endif
