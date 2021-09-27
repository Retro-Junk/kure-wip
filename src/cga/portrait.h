#ifndef _PORTRAIT_H_
#define _PORTRAIT_H_

extern byte *cur_image_pixels;
extern byte cur_image_size_w;
extern byte cur_image_size_h;
extern byte cur_image_coords_x;
extern byte cur_image_coords_y;
extern unsigned int cur_image_offs;
extern unsigned int cur_image_end;
extern byte cur_image_idx;
extern byte cur_image_anim1;
extern byte cur_image_anim2;
extern unsigned int cur_frame_width;

extern volatile byte vblank_ticks;

int DrawPortrait(byte **desc, byte *x, byte *y, byte *width, byte *height);
void AnimPortrait(byte layer, byte index, byte delay);

void DrawBoxAroundSpot(void);

void MergeImageAndSpriteData(byte *target, signed int pitch, byte *source, unsigned int w, unsigned int h);
void MergeImageAndSpriteDataFlip(byte *target, signed int pitch, byte *source, unsigned int w, unsigned int h);

void BlinkToRed(void);
void BlinkToWhite(void);


#endif
