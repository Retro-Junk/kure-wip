#include <dos.h>
#ifdef __WATCOMC__
#include <i86.h>
#include <conio.h>
#endif
#include <string.h>

#include "common.h"
#include "ega.h"
#include "cursor.h"
#include "resdata.h"
#include "print.h"


/*
EGA programming notes
Video memory starts at 0xA000:0 .
Each byte represent 8 consecutive pixels on screen (most significant bit - leftmost pixel.)
Different colors works by splitting pixel's palette entry index onto individual bits and store each on its own bitplane.
So, 16-color graphics = 4 bits per palette index = 4 bitplanes.
All bitplanes starts at the same address (0xA000:0) and can be selected via port xxx , register #xx.
It's possible to select multiple bitplanes, this means write operation will send input byte to selected bitplanes, leaving other unaffected.
There's intermediate 8-bit buffer (aka latch) for each bitplane.
Buffers are refilled when a read operation for corresponding memory location is issued.
Buffer's values can be written back to video memory, fully or partially overriden by new input values.
Map mask register selects which bitplanes will be modified
Bit mask selects wich bits of new input will replace corresponding bits of a latch and then written to the memory
Another logic operations between latch's and input bits are also possible.
It is possible to use "linear" mode, where input byte will be automatically split between all planes, but in-plane bit index need to be selected first.


*/

extern byte backbuffer[320u * 200 / 8 * 4];

byte carpc_data[RES_CARPC_MAX];

extern byte *scratch_mem2;

byte ega_busy = 1;

cursor_params_t ega_cursor_params_page0;
cursor_params_t ega_cursor_params_page1;
cursor_params_t *ega_cursor_params = &ega_cursor_params_page1;

volatile unsigned char *frontbuffer_scrn = EGA_PAGE0;
volatile unsigned char *frontbuffer_work = EGA_PAGE1;

uint16 ega_char_draw_color = 0;

void EGA_SetRegValue(unsigned short port, byte index, byte value) {
	outport(port, (value << 8) | index);
}

void EGA_SetMapMask(byte mask) {
	EGA_SetRegValue(EGA_SEQUENCER_PORT, EGA_SEQUENCER_REG_MAPMASK, mask);
}

void EGA_WaitVBlank(void) {
	while ((inportb(0x3DA) & 8) == 0) ;
	while ((inportb(0x3DA) & 8) != 0) ;
}

/*standard EGA palette, dark-gray is replaced by black*/
static byte ega_palette[17] = {0, 1, 2, 3, 4, 5, 20, 7, 0, 57, 58, 59, 60, 61, 62, 63, 0};

/*
Set 17-color EGA palette
*/
void EGA_SetPalette(byte palette[17]) {
	struct REGPACK reg;
	memset(&reg, 0, sizeof(reg));
#ifdef __386__
	reg.r_ax = 0x1002;
#else
	reg.r_ax = 0x1002;
#endif
	reg.r_es = FP_SEG(palette);
	reg.r_dx = FP_OFF(palette);
	intr(0x10, &reg);
}


/*
  Switch to EGA 320x200x4bpp mode
*/
void EGA_SwitchToGraphicsMode(void) {
#ifdef __386__
	union REGS reg;
	reg.w.ax = 0x0D;
	int386(0x10, &reg, &reg);
#else
	union REGS reg;
	reg.x.ax = 0x0D;
	int86(0x10, &reg, &reg);
#endif
	/*set display start address at video memory address 0*/
	outport(0x3D4, 0x000C);
	outport(0x3D4, 0x000D);

	EGA_SetPalette(ega_palette);
}

/*
  Switch to text mode
*/
void EGA_SwitchToTextMode(void) {
#ifdef __386__
	union REGS reg;
	reg.w.ax = 3;
	int386(0x10, &reg, &reg);
#else
	union REGS reg;
	reg.x.ax = 3;
	int86(0x10, &reg, &reg);
#endif
}


/*
Initialize EGA controller to Mode 0
*/
void EGA_SetupPorts(void) {
	outport(0x3CE, 5);		/*set read mode to 0*/
	outport(0x3CE, 1);		/*disable set/reset for all planes*/
	outport(0x3CE, 3);		/*disable bit rotate and logic ops*/
	outport(0x3CE, 0xFF08);	/*set bit mask to FF (use all input bits)*/
}

void EGA_SetMode2(void) {
	EGA_SetMapMask(0xF);
	EGA_SetRegValue(EGA_GC_PORT, 5, 2);	/*set write mode 2*/
	EGA_SetRegValue(EGA_GC_PORT, 3, 0);	/*disable data rotate/func*/
}

typedef struct backuphdr_t {
byte	w;
byte	h;
uint16	ofs;
byte	bitofs;
byte	data[0];
} backuphdr_t;


/*
Calc video buffer offset from raw x and y coordinates.
Return in-byte offset bit in bitofs
*/
uint16 EGA_CalcXY(uint16 x, byte y, byte *bitofs) {
	uint16 ofs = EGA_BYTES_PER_LINE * y + x / 8;
	*bitofs = 0x80u >> (x % 8);
	return ofs;	
}

/*
Calc video buffer offset from packed x and raw y coordinates.
Return in-byte offset bit in bitofs
*/
uint16 EGA_CalcXY_p(byte x, byte y, byte *bitofs) {
	uint16 ofs = EGA_BYTES_PER_LINE * y + x / 2;
	*bitofs = (x % 2) ? 8 : 0x80;
	return ofs;	
}

/*
Read screen memory to a backup buffer

w - packed width of image (= real/4)
bitofs - start pixel in byte (as a mask)

Screen data is stored as columns[plane][w][h] , where w is padded byte-width of raster

Returns next memory location after stored backup data
*/
byte *EGA_BackupImage(byte *buffer, byte *screen, byte w, byte h, uint16 ofs, byte bitofs) {
	int plane, i, j;

	backuphdr_t *hdr = (backuphdr_t*)buffer;
	hdr->w = w;
	hdr->h = h;
	hdr->ofs = ofs;
	hdr->bitofs = bitofs;
	buffer = hdr->data;
	ega_busy |= 1;

	/*if not starts at byte boundary, store extra byte*/
	if (bitofs != 0x80)
		w++;
	w = (w / 2) + (w % 2);	/*w = (w + 1) / 2*/

	outport(0x3CE, 5);			/*set read mode to 0*/
	outport(0x3CE, 1);			/*disable set/reset for all planes*/
	outport(0x3CE, 3);			/*disable bit rotate and logic ops*/
	outport(0x3CE, 0xFF08);	/*set bit mask to FF (use all input bits)*/

	for (plane = 0;plane < 4;plane++) {
		uint16 o = ofs;
		EGA_SetRegValue(EGA_GC_PORT, 4, plane);	/*set read bitplane index*/
		/*store whole bitplane*/
		for (i = 0;i < w;i++) {
			uint16 oo = ofs;
			/*store whole column*/
			for (j = 0;j < h;j++) {
				*buffer++ = screen[ofs];
				ofs += 40;
			}
			ofs = oo + 1;
		}
		ofs = o;
	}

	ega_busy &= ~1;
	return buffer;
}

byte *EGA_BackupImageScrn(unsigned int ofs, unsigned int bitofs, unsigned int w, unsigned int h) {
	return EGA_BackupImage(scratch_mem2, frontbuffer_scrn, w, h, ofs, bitofs);
}

/*
Restore saved image to target screen buffer
*/
void EGA_RestoreImage(byte *buffer, volatile byte *page) {
	int plane, i, j;
	byte maskl, maskr;
	unsigned int w, h;
	uint16 ofs;
	byte bitofs;
	backuphdr_t *hdr = (backuphdr_t*)buffer;

	if (!buffer)
		return 0;

	w = hdr->w;
	h = hdr->h;
	ofs = hdr->ofs;
	bitofs = hdr->bitofs;
	buffer = &hdr->data;

	ega_busy |= 1;
	EGA_SetupPorts();

	maskl = 0xFF;
	if (bitofs != 0x80) {
		maskl = 0x0F;
		w++;
	}

	maskr = 0xFF;
	if (w % 2) {
		maskr = 0xF0;
		w += 2;
	}

	w /= 2;

	for (plane = 0;plane < 4;plane++) {
		uint16 o = ofs;
		EGA_SetMapMask(1 << plane);				/*set write plane mask*/
		EGA_SetRegValue(EGA_GC_PORT, 4, plane);	/*set read bitplane index*/
		for (i = 0;i < w;i++) {
			uint16 oo = ofs;
			byte mask = 0xFF;
			if (i == 0)
				mask = maskl;
			else if (i == w - 1)
				mask = maskr;

			for (j = 0;j < h;j++) {
				byte t = *buffer++;
				page[ofs] &= ~mask;
				page[ofs] |= t & mask;
				ofs += EGA_BYTES_PER_LINE;
			}
			ofs = oo + 1;
		}
		ofs = o;
	}

	ega_busy &= ~1;
	return;
}

void EGA_RestoreImageScrn(byte *buffer) {
	EGA_RestoreImage(buffer, frontbuffer_scrn);
}

void EGA_RestoreImageWork(byte *buffer) {
	EGA_RestoreImage(buffer, frontbuffer_work);
}

/*
Restore saved image from scratch mem to target screen buffer
*/
void EGA_RestoreBackupImage(volatile byte *page) {
	EGA_RestoreImage(scratch_mem2, page);
}

void EGA_RestoreBackupImageBoth(void) {
	EGA_RestoreImageWork(scratch_mem2);
	EGA_RestoreImageScrn(scratch_mem2);
}

/*
Flip screen pages
*/
void EGA_Flip(void) {
	volatile unsigned char *t;
	if (frontbuffer_scrn == EGA_PAGE1) {
		/*switch screen to page 0 at 0000*/
		EGA_SetRegValue(EGA_CRTC_PORT, 0xC, EGA_PAGE0_OFS >> 8);
		EGA_SetRegValue(EGA_CRTC_PORT, 0xD, EGA_PAGE0_OFS & 255);
		ega_cursor_params = &ega_cursor_params_page0;
	} else {
		/*switch screen to page 1 at 8000*/
		EGA_SetRegValue(EGA_CRTC_PORT, 0xC, EGA_PAGE1_OFS >> 8);
		EGA_SetRegValue(EGA_CRTC_PORT, 0xD, EGA_PAGE1_OFS & 255);
		ega_cursor_params = &ega_cursor_params_page1;
	}
	t = frontbuffer_work;
	frontbuffer_work = frontbuffer_scrn;
	frontbuffer_scrn = t;
	EGA_WaitVBlank();
}

/*
Copy from conventional memory source to EGA bitplanes target
*/
void EGA_CopyToPlanes(byte *source, byte *target) {
	EGA_SetupPorts();
	EGA_SetMapMask(1);
	memcpy(target, source, 8000); source += 8000;
	EGA_SetMapMask(2);
	memcpy(target, source, 8000); source += 8000;
	EGA_SetMapMask(4);
	memcpy(target, source, 8000); source += 8000;
	EGA_SetMapMask(8);
	memcpy(target, source, 8000);
}

/*
Copy from EGA bitplanes source to conventional memory target
*/
void EGA_CopyFromPlanes(byte *source, byte *target) {
	EGA_SetupPorts();
	EGA_SetRegValue(EGA_GC_PORT, 4, 0);
	memcpy(target, source, 8000); target += 8000;
	EGA_SetRegValue(EGA_GC_PORT, 4, 1);
	memcpy(target, source, 8000); target += 8000;
	EGA_SetRegValue(EGA_GC_PORT, 4, 2);
	memcpy(target, source, 8000); target += 8000;
	EGA_SetRegValue(EGA_GC_PORT, 4, 3);
	memcpy(target, source, 8000);
}

/*
Copy whole backbuffer to the target screen (page)
*/
void EGA_BackBufferToRealFullPage(byte *target) {
	ega_busy |= 1;
	EGA_CopyToPlanes(backbuffer, target);
	ega_busy &= ~1;
}

/*
Copy whole backbuffer to the work screen page
*/
void EGA_BackBufferToRealFull(void) {
	EGA_BackBufferToRealFullPage(frontbuffer_work);
}

/*
Copy whole backbuffer to the all screen pages
*/
void EGA_BackToRealBothPages(void) {
	EGA_BackBufferToRealFull();
	EGA_Flip();
	EGA_BackBufferToRealFull();
}

void EGA_RealToBackBuffer(void) {
	ega_busy |= 1;




	ega_busy &= ~1;
}


/*
Copy from conventional memory source to EGA bitplanes target, byte-aligned
*/
void EGA_BlitBytes(byte *source, uint16 w, uint16 h, byte *target, uint16 ofs) {
	uint16 y, o;
	byte plane;

	ega_busy |= 1;
	EGA_SetupPorts();
	for (plane = 1;plane < (1<<4);plane <<= 1) {
		EGA_SetMapMask(plane);
		o = ofs;
		for(y = 0;y < h;y++) {
			memcpy(target + o, source, w);
			source += w;
			o += EGA_BYTES_PER_LINE;
		}
	}
	ega_busy &= ~1;
}

/*
Copy from EGA bitplanes source to conventional memory buffer, byte-aligned
*/
void EGA_GrabBytesInt(byte *target, uint16 w, uint16 h, byte *source, uint16 ofs) {
	uint16 y, o;
	byte plane;

	for (plane = 0;plane < 4;plane += 1) {
		EGA_SetRegValue(EGA_GC_PORT, 4, plane);
		o = ofs;
		for(y = 0;y < h;y++) {
			memcpy(target, source + o, w);
			o += EGA_BYTES_PER_LINE;
			target += w;
		}
	}
}

/*
Copy from EGA bitplanes source to conventional memory buffer, byte-aligned
*/
void EGA_GrabBytes(byte *target, uint16 w, uint16 h, byte *source, uint16 ofs) {
	ega_busy |= 1;
	EGA_SetupPorts();
	EGA_GrabBytesInt(target, w, h, source, ofs);
	ega_busy &= ~1;
}


/*
Copy image's real screen data to backbuffer
*/
void EGA_RefreshImageData(byte *buffer) {
	unsigned int w, h;
	uint16 ofs;

	if (!buffer)
		return;

	h = *(byte *)(buffer + 0);
	w = *(byte *)(buffer + 1);
	ofs = *(uint16 *)(buffer + 2);

	/*CGA_CopyScreenBlock(CGA_SCREENBUFFER, w, h, backbuffer, ofs);*/
}

void EGA_DrawCursor(void) {
	byte t;
	uint16 x, y, o;
	byte *pixels;
	ega_busy |= 1;
	EGA_SetupPorts();
	EGA_GrabBytesInt(ega_cursor_params->backup, 24/8, 16, frontbuffer_work, ega_cursor_params->drawoffs);
	ega_cursor_params->restoffs = ega_cursor_params->drawoffs;

	EGA_SetMode2();

	pixels = sprit_load_buffer;
	o = ega_cursor_params->drawoffs;
	for (y = 0;y < 16;y++) {
		for (x = 0;x < 24/8;x++) {
			EGA_SetRegValue(EGA_GC_PORT, 8, *pixels++);	/*set write mask*/
			t = frontbuffer_work[o + x];				/*latch*/
			frontbuffer_work[o + x] = 0;				/*clear*/
			EGA_SetRegValue(EGA_GC_PORT, 8, *pixels++);	/*set write mask*/
			t = frontbuffer_work[o + x];				/*latch*/
			frontbuffer_work[o + x] = cursor_color;		/*write*/
		}
		o += EGA_BYTES_PER_LINE;
	}

	ega_busy &= ~1;
}

/*
Restore pixels under cursor in work buffer
*/
void EGA_UndrawCursorWork(void) {
	EGA_BlitBytes(ega_cursor_params->backup, 24/8, 16, frontbuffer_work, ega_cursor_params->restoffs);
}

/*
Restore pixels under cursor in all buffer
*/
void EGA_UndrawCursorBoth(void) {
	EGA_UndrawCursorWork();
	EGA_Flip();
	EGA_UndrawCursorWork();
}


/*
Load and uncompress 4-bit sprite
Return next ptr after the loaded sprite
*/
byte *EGA_LoadSprite(byte index, byte *bank, byte *buffer, byte header_only) {
	byte w, h;
	unsigned int rsize;
	byte *sprite, *sprite_end;
	sprite = SeekToEntryW(bank, index, &sprite_end);
	w = *sprite++;
	h = *sprite++;
	rsize = w * h * 4 / 2;  /*raster part size*/

	*buffer++ = w;
	*buffer++ = h;
	if (header_only) {
		memset(buffer, 8, rsize * 2);   /*wipe pixels*/
		buffer += rsize * 2;
	} else {
		/*solid sprite*/
		while (rsize--) {
			byte px = *sprite++;
			*buffer++ = px >> 4;
			*buffer++ = px & 15;
		}
	}
	return buffer;
}

byte sprit_load_buffer[1290];

byte *LoadSprit(byte index) {
	EGA_LoadSprite(index, sprit_data + 4, sprit_load_buffer, 0);
	return sprit_load_buffer;
}

byte *LoadPersSprit(byte index) {
	/*Use single large chunk for pers1+pers2*/
	EGA_LoadSprite(index, perso_data + 4, scratch_mem2, 0);
	return scratch_mem2;
}

/*
Draw a vertical line with origin x:y and length l, using color
NB! Line must not wrap around the edge
*/
void EGA_DrawVLine(unsigned int x, unsigned int y, unsigned int l, byte color, volatile byte *target) {
	unsigned int ofs;
	byte bitofs;
	ega_busy |= 1;

	ofs = EGA_CalcXY(x, y, &bitofs);

	EGA_SetMode2();
	EGA_SetRegValue(EGA_GC_PORT, 8, bitofs);	/*select write bit*/

	while (l--) {
		byte t;
		t = target[ofs];			/*latch*/
		target[ofs] = color;		/*write*/
		ofs += EGA_BYTES_PER_LINE;
	}

	ega_busy &= ~1;
}

/*
Draw a horizontal line with origin x:y and length l, using color
NB! Line must not wrap around the edge
*/
void EGA_DrawHLine(unsigned int x, unsigned int y, unsigned int l, byte color, volatile byte *target) {
	unsigned int ofs;
	byte bitofs;
	ega_busy |= 1;

	ofs = EGA_CalcXY(x, y, &bitofs);

	EGA_SetMode2();

	while (l--) {
		byte t;
		EGA_SetRegValue(EGA_GC_PORT, 8, bitofs);	/*select write bit*/

		t = target[ofs];			/*latch*/
		target[ofs] = color;		/*write*/

		bitofs >>= 1;
		/*all byte's pixels done?*/
		if (bitofs == 0) {
			ofs++;
			bitofs = 0x80;
		}
	}
	ega_busy &= ~1;
}

void EGA_SetDrawColor(uint16 color) {
	ega_char_draw_color = color;
}

/*
Print a character at current cursor pos, then advance
*/
void EGA_PrintChar(byte c, volatile byte *target) {
	unsigned int i;
	byte *font = carpc_data + c * EGA_FONT_HEIGHT;
	byte bitofs;
	unsigned int ofs = EGA_CalcXY_p(char_draw_coords_x++, char_draw_coords_y, &bitofs);
	bitofs = (bitofs == 0x80) ? 4 : 0;

	ega_busy |= 1;
	EGA_SetMode2();

	for (i = 0; i < EGA_FONT_HEIGHT; i++) {
		byte t;

		c = *font++;

		c = (c >> (8 - bitofs)) | (c << bitofs);	/*rotate nibbles*/
		EGA_SetRegValue(EGA_GC_PORT, 8, c);	/*select write bits*/
		t = target[ofs];			/*latch*/
		target[ofs] = ega_char_draw_color & 255;		/*write*/

		c = ~c;
		c &= 0x0F << bitofs;
		EGA_SetRegValue(EGA_GC_PORT, 8, c);	/*select write bits*/
		t = target[ofs];			/*latch*/
		target[ofs] = ega_char_draw_color >> 8;		/*write*/

		ofs += EGA_BYTES_PER_LINE;
	}

	ega_busy &= ~1;
}


void EGA_MergePartialColumn(byte mask, byte h, volatile byte *source, volatile byte *target, uint16 ofs) {
	byte plane;
#if 1
	EGA_SetupPorts();
#else
	EGA_SetRegValue(EGA_GC_PORT, 8, 0xFF);	/*set write mask*/
	EGA_SetRegValue(EGA_GC_PORT, 5, 0);		/*set write mode 0*/
	EGA_SetRegValue(EGA_GC_PORT, 1, 0);		/*disable set/reset*/
	EGA_SetRegValue(EGA_GC_PORT, 3, 0);		/*disable data rotate/func*/
#endif

	for (plane = 0;plane < 4;plane++) {
		byte y;
		uint16 o = ofs;
		EGA_SetMapMask(1 << plane);				/*set write bitplane*/
		EGA_SetRegValue(EGA_GC_PORT, 4, plane);	/*set read bitplane*/
		for (y = 0;y < h;y++) {
			byte t = source[o];
			target[o] &= ~mask;
			target[o] |= t & mask;
			o += EGA_BYTES_PER_LINE;
		}
	}
}

/*
Copy pixels from one EGA page to another, nibble-aligned
*/
void EGA_MergePages(byte w, byte h, volatile byte *source, volatile byte *target, uint16 ofs, byte bitofs) {
	ega_busy |= 1;

	if (bitofs != 0x80) {
		EGA_MergePartialColumn(0xF, h, source, target, ofs);
		w--;
		ofs++;
	}

	if (w / 2 != 0) {
		byte x, y;
		uint16 o = ofs;

		EGA_SetRegValue(EGA_GC_PORT, 5, 1);
		EGA_SetMapMask(0xF);

		for (y = 0;y < h;y++) {
			for (x = 0;x < w / 2;x++)
				target[o + x] = source[o + x];
			o += EGA_BYTES_PER_LINE;
		}

		ofs += w / 2;
	}

	if (w % 2) {
		EGA_MergePartialColumn(0xF0, h, source, target, ofs);
	}

	ega_busy &= ~1;
}

void EGA_MergeScrnToWork(byte w, byte h, uint16 ofs, byte bitofs) {
	EGA_MergePages(w, h, frontbuffer_scrn, frontbuffer_work, ofs, bitofs);
}

void EGA_MergeWorkToScrn(byte w, byte h, uint16 ofs, byte bitofs) {
	EGA_MergePages(w, h, frontbuffer_work, frontbuffer_scrn, ofs, bitofs);
}

void EGA_BlitSpriteData(byte *sprdata, byte rw, byte w, byte h, volatile byte *page, uint16 ofs, byte bitofs) {
	int x, y;
	ega_busy |= 1;
	EGA_SetMode2();
	for (x = 0;x < w * 4;x++) {
		uint16 o = ofs;
		byte *spr = sprdata;
		EGA_SetRegValue(EGA_GC_PORT, 8, bitofs);	/*select write plane*/
		for (y = 0;y < h;y++) {
			byte pix = *spr;
			spr += rw;
			if (pix != 0) {
				byte t = page[o];	/*latch*/
				page[o] = pix;
			}
			o += EGA_BYTES_PER_LINE;
		}
		sprdata++;

		bitofs >>= 1;
		/*all byte's pixels done?*/
		if (bitofs == 0) {
			ofs++;
			bitofs = 0x80;
		}
	}
	ega_busy &= ~1;
}

byte last_sprite_w;
byte last_sprite_h;
uint16 last_sprite_ofs;
byte last_sprite_bitofs;

void EGA_ShowSprite(byte index, byte x, byte y, volatile byte *page) {
	byte w, h;
	unsigned int ofs;
	byte bitofs;
	byte *sprite = LoadSprit(index);
	ofs = EGA_CalcXY_p(x, y, &bitofs);
	w = sprite[0];
	h = sprite[1];
	EGA_BlitSpriteData(sprite + 2, w * 4, w, h, page, ofs, bitofs);
	last_sprite_w = w;
	last_sprite_h = h;
	last_sprite_ofs = ofs;
	last_sprite_bitofs = bitofs;
}

void EGA_ShowSpriteScrn(byte index, byte x, byte y) {
	EGA_ShowSprite(index, x, y, frontbuffer_scrn);
}

void EGA_ShowSpriteWork(byte index, byte x, byte y) {
	EGA_ShowSprite(index, x, y, frontbuffer_work);
}

void EGA_BackupAndShowSprite(byte index, byte x, byte y) {
	EGA_ShowSprite(index, x, y, frontbuffer_work);
	EGA_BackupImageScrn(last_sprite_ofs, last_sprite_bitofs, last_sprite_w, last_sprite_h);
}
