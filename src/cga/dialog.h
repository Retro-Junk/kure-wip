#ifndef _DIALOG_H_
#define _DIALOG_H_

extern unsigned int cur_str_index;
extern unsigned int cur_dlg_index;

enum DirtyRectKind {
	DirtyRectFree = 0,
	DirtyRectBubble = 1,    /*bubble with spike*/
	DirtyRectSprite = 2,    /*portrait*/
	DirtyRectText = 3       /*text bubble w/o spike*/
};

typedef struct dirty_rect_t {
	byte kind;
	unsigned int offs;
	byte height;
	byte width;
	byte y;        /*for DirtyRectBubble this is spike offs*/
	byte x;
} dirty_rect_t;

#define MAX_DIRTY_RECT 10
extern dirty_rect_t dirty_rects[];
extern dirty_rect_t *last_dirty_rect;

#define SPIKE_MASK     0xE0
#define SPIKE_UPLEFT   0
#define SPIKE_UPRIGHT  0x20
#define SPIKE_DNRIGHT  0x80
#define SPIKE_DNLEFT   0xA0
#define SPIKE_BUBBLES  0x40
#define SPIKE_BUBRIGHT 0xC0
#define SPIKE_BUBLEFT  0xE0

void AddDirtyRect(byte kind, byte x, byte y, byte w, byte h, unsigned int offs);
void GetDirtyRectAndFree(int index, byte *kind, byte *x, byte *y, byte *w, byte *h, unsigned int *offs);
void GetDirtyRectAndSetSprite(int index, byte *kind, byte *x, byte *y, byte *w, byte *h, unsigned int *offs);

void PopDirtyRects(byte kind);
void DrawPersonBubble(byte x, byte y, byte flags, byte *msg);
void DesciTextBox(unsigned int x, unsigned int y, unsigned int width, byte *msg);

void PromptWait(void);

byte *SeekToString(byte *bank, unsigned int num);
byte *SeekToStringScr(byte *bank, unsigned int num, byte **ptr);

#endif
