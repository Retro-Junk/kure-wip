#ifndef _CGA_H_
#define _CGA_H_

#include <dos.h>

#define CGA_WIDTH 320
#define CGA_HEIGHT 200
#define CGA_BASE_SEG 0xB800
#define CGA_ODD_LINES_OFS 0x2000
#define CGA_BITS_PER_PIXEL 2
#define CGA_PIXELS_PER_BYTE (8 / CGA_BITS_PER_PIXEL)
#define CGA_BYTES_PER_LINE (CGA_WIDTH / CGA_PIXELS_PER_BYTE)

#ifdef __386__
#define CGA_SCREENBUFFER ((byte*)(CGA_BASE_SEG * 16))
#else
#define CGA_SCREENBUFFER ((byte*)MK_FP(CGA_BASE_SEG, 0))
#endif

#define CGA_FONT_HEIGHT 6

#define CGA_NEXT_LINE(offs) ((CGA_ODD_LINES_OFS ^ (offs)) + (((offs) & CGA_ODD_LINES_OFS) ? 0 : CGA_BYTES_PER_LINE))
#define CGA_PREV_LINE(offs) ((CGA_ODD_LINES_OFS ^ (offs)) - (((offs) & CGA_ODD_LINES_OFS) ? CGA_BYTES_PER_LINE : 0))

#define frontbuffer CGA_SCREENBUFFER
extern byte backbuffer[0x4000];

extern byte sprit_load_buffer[1290];

extern byte cga_pixel_flip[256];

extern byte char_draw_coords_x;
extern byte char_draw_coords_y;
extern byte *char_xlat_table;
extern byte string_ended;
extern byte char_draw_max_width;
extern byte char_draw_max_height;

void SwitchToGraphicsMode(void);
void SwitchToTextMode(void);

void WaitVBlank(void);

void CGA_ColorSelect(byte csel);
void CGA_BackBufferToRealFull(void);
void CGA_RealBufferToBackFull(void);
void CGA_SwapRealBackBuffer(void);

void CGA_SwapScreenRect(byte *pixels, unsigned int w, unsigned int h, byte *screen, unsigned int ofs);

unsigned int CGA_CalcXY(unsigned int x, unsigned int y);
unsigned int CGA_CalcXY_p(unsigned int x, unsigned int y);

void CGA_CopyScreenBlock(byte *source, unsigned int w, unsigned int h, byte *target, unsigned int ofs);

byte *CGA_BackupImage(byte *source, unsigned int ofs, unsigned int w, unsigned int h, byte *buffer);
byte *CGA_BackupImageReal(unsigned int ofs, unsigned int w, unsigned int h);

void CGA_RestoreImage(byte *buffer, byte *target);
void CGA_RefreshImageData(byte *buffer);
void CGA_RestoreBackupImage(byte *target);

void CGA_Blit(byte *pixels, unsigned int pw, unsigned int w, unsigned int h, byte *screen, unsigned int ofs);
void CGA_BlitAndWait(byte *pixels, unsigned int pw, unsigned int w, unsigned int h, byte *screen, unsigned int ofs);
void CGA_FillAndWait(byte pixel, unsigned int w, unsigned int h, byte *screen, unsigned int ofs);

void CGA_DrawVLine(unsigned int x, unsigned int y, unsigned int l, byte color, byte *target);
void CGA_DrawHLine(unsigned int x, unsigned int y, unsigned int l, byte color, byte *target);
unsigned int CGA_DrawHLineWithEnds(uint16 bmask, uint16 bpix, byte color, unsigned int l, byte *target, unsigned int ofs);

void CGA_PrintChar(byte c, byte *target);

void CGA_BlitScratchBackSprite(unsigned int sprofs, unsigned int w, unsigned int h, byte *screen, unsigned int ofs);
void CGA_BlitFromBackBuffer(byte w, byte h, byte *screen, unsigned int ofs);

void CGA_BlitSprite(byte *pixels, signed int pw, unsigned int w, unsigned int h, byte *screen, unsigned int ofs);
void CGA_BlitSpriteFlip(byte *pixels, signed int pw, unsigned int w, unsigned int h, byte *screen, unsigned int ofs);

void CGA_BlitSpriteBak(byte *pixels, signed int pw, unsigned int w, unsigned int h, byte *screen, unsigned int ofs, byte *backup, byte mask);

void DrawSprite(byte *sprite, byte *screen, unsigned int ofs);
void DrawSpriteFlip(byte *sprite, byte *screen, unsigned int ofs);

void DrawSpriteN(byte index, unsigned int x, unsigned int y, byte *target);
void DrawSpriteNFlip(byte index, unsigned int x, unsigned int y, byte *target);

void BackupAndShowSprite(byte index, byte x, byte y);

byte *LoadSprite(byte index, byte *bank, byte *buffer, byte header_only);

byte *LoadSprit(byte index);
byte *LoadPersSprit(byte index);

void CGA_AnimLiftToUp(byte *pixels, unsigned int pw, unsigned int w, unsigned int h, byte *screen, unsigned int x, unsigned int y);
void CGA_AnimLiftToDown(byte *pixels, unsigned int pw, unsigned int w, unsigned int h, byte *screen, unsigned int ofs);
void CGA_AnimLiftToLeft(unsigned int n, byte *pixels, unsigned int pw, unsigned int w, unsigned int h, byte *screen, unsigned int ofs);
void CGA_AnimLiftToRight(unsigned int n, byte *pixels, unsigned int pw, unsigned int w, unsigned int h, byte *screen, unsigned int ofs);

void CGA_HideScreenBlockLiftToUp(unsigned int n, byte *screen, byte *source, unsigned int w, unsigned int h, byte *target, unsigned int ofs);
void CGA_HideScreenBlockLiftToDown(unsigned int n, byte *screen, byte *source, unsigned int w, unsigned int h, byte *target, unsigned int ofs);
void CGA_HideScreenBlockLiftToLeft(unsigned int n, byte *screen, byte *source, unsigned int w, unsigned int h, byte *target, unsigned int ofs);
void CGA_HideScreenBlockLiftToRight(unsigned int n, byte *screen, byte *source, unsigned int w, unsigned int h, byte *target, unsigned int ofs);

void CGA_HideShatterFall(byte *screen, byte *source, unsigned int w, unsigned int h, byte *target, unsigned int ofs);

void CGA_TraceLine(unsigned int sx, unsigned int ex, unsigned int sy, unsigned int ey, byte *source, byte *target);

void CGA_ZoomImage(byte *pixels, byte w, byte h, byte nw, byte nh, byte *target, unsigned int ofs);
void CGA_AnimZoomIn(byte *pixels, byte w, byte h, byte *target, unsigned int ofs);

void CGA_ZoomInplaceXY(byte *pixels, byte w, byte h, byte nw, byte nh, unsigned int x, unsigned int y, byte *target);

#endif
