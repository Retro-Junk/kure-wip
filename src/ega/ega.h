#ifndef _EGA_H_
#define _EGA_H_

#define EGA_WIDTH 320
#define EGA_HEIGHT 200
#define EGA_BITS_PER_PIXEL 1
#define EGA_PIXELS_PER_BYTE (8 / EGA_BITS_PER_PIXEL)

/*single bitplane size*/
#define EGA_PLANE_SIZE (EGA_WIDTH*EGA_HEIGHT/8)
#define EGA_BYTES_PER_LINE (EGA_WIDTH / EGA_PIXELS_PER_BYTE)


/*logical page size (must be at least bitplane size*/
#define EGA_PAGE0_OFS 0x0000u
#define EGA_PAGE1_OFS 0x8000u

#define EGA_BASE_SEG 0xA000u

#define EGA_PAGE0_SEG (EGA_BASE_SEG + (EGA_PAGE0_OFS / 16))
#define EGA_PAGE1_SEG (EGA_BASE_SEG + (EGA_PAGE1_OFS / 16))

#define EGA_PAGE0 ((unsigned char*)MK_FP(EGA_BASE_SEG, EGA_PAGE0_OFS))
#define EGA_PAGE1 ((unsigned char*)MK_FP(EGA_BASE_SEG, EGA_PAGE1_OFS))

#define EGA_SEQUENCER_PORT 0x3C4
#define EGA_SEQUENCER_REG_MAPMASK 2

#define EGA_CRTC_PORT 0x3D4

#define EGA_GC_PORT 0x3CE

#define EGA_FONT_HEIGHT 6

typedef struct cursor_params_t {
	uint16	restoffs;
	uint16	drawoffs;
	byte	backup[(24 * 16 / EGA_PIXELS_PER_BYTE) * 4];
} cursor_params_t;

extern cursor_params_t *ega_cursor_params;

extern byte ega_busy;

extern volatile unsigned char *frontbuffer_scrn;
extern volatile unsigned char *frontbuffer_work;

extern byte sprit_load_buffer[1290];

void EGA_SwitchToGraphicsMode(void);
void EGA_SwitchToTextMode(void);


uint16 EGA_CalcXY(uint16 x, byte y, byte *bitofs);
uint16 EGA_CalcXY_p(byte x, byte y, byte *bitofs);

void EGA_Flip(void);

void EGA_BackBufferToRealFull(void);
void EGA_BackToRealBothPages(void);

void EGA_MergePages(byte w, byte h, volatile byte *source, volatile byte *target, uint16 ofs, byte bitofs);
void EGA_MergeScrnToWork(byte w, byte h, uint16 ofs, byte bitofs);
void EGA_MergeWorkToScrn(byte w, byte h, uint16 ofs, byte bitofs);


void EGA_UndrawCursorWork(void);
void EGA_UndrawCursorBoth(void);

void EGA_DrawVLine(unsigned int x, unsigned int y, unsigned int l, byte color, volatile byte *target);
void EGA_DrawHLine(unsigned int x, unsigned int y, unsigned int l, byte color, volatile byte *target);

void EGA_SetDrawColor(uint16 color);
void EGA_PrintChar(byte c, volatile byte *target);

extern byte last_sprite_w;
extern byte last_sprite_h;
extern uint16 last_sprite_ofs;
extern byte last_sprite_bitofs;

byte *EGA_LoadSprite(byte index, byte *bank, byte *buffer, byte header_only);

void EGA_ShowSprite(byte index, byte x, byte y, volatile byte *page);

void EGA_ShowSpriteScrn(byte index, byte x, byte y);
void EGA_ShowSpriteWork(byte index, byte x, byte y);

extern byte last_sprite_w;
extern byte last_sprite_h;
extern uint16 last_sprite_ofs;
extern byte last_sprite_bitofs;

void EGA_BackupAndShowSprite(byte index, byte x, byte y);

byte *EGA_BackupImageScrn(unsigned int ofs, unsigned int bitofs, unsigned int w, unsigned int h);
void EGA_RestoreBackupImageBoth(void);


void EGA_DrawCursor(void);
void EGA_UndrawCursorWork(void);
void EGA_UndrawCursorBoth(void);

#endif
