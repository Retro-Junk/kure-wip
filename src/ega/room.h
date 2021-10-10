#ifndef _ROOM_H_
#define _ROOM_H_

#define SPOTFLG_1  0x01
#define SPOTFLG_2  0x02
#define SPOTFLG_8  0x08
#define SPOTFLG_10 0x10
#define SPOTFLG_20 0x20
#define SPOTFLG_40 0x40
#define SPOTFLG_80 0x80

/*static room object*/
/*TODO: manipulated from script, do not change*/
typedef struct spot_t {
	byte sx;
	byte ex;
	byte sy;
	byte ey;
	byte flags;
	byte hint;
	uint16 command;
} spot_t;

#define PERSFLAGS  0xF0
#define PERSFLG_10 0x10
#define PERSFLG_20 0x20
#define PERSFLG_40 0x40
#define PERSFLG_80 0x80

/*person*/
/*TODO: manipulated from script, do not change*/
typedef struct pers_t {
	byte area;     /*location*/
	byte flags;    /*flags in bits 7..4 and room index in bits 3..0*/
	byte name;     /*name index*/
	byte index;    /*animations index (in lutins_table) in bits 7..3 , spot index in bits 2..0*/
	byte item;     /*inventory item index (1-based)*/
} pers_t;

#define ANIMFLG_USESPOT 0x80

typedef struct animdesc_t {
	byte index;    /*flag in bit 7, animation index in bits 6..0*/
	union {
		struct {
			byte x, y;
		} coords;
		uint16 desc;
	} params;
} animdesc_t;

typedef struct vortanims_t {
	byte room;
	animdesc_t field_1;
	animdesc_t field_4;
	animdesc_t field_7;
	animdesc_t field_A;
} vortanims_t;

typedef struct turkeyanims_t {
	byte room;
	animdesc_t field_1;
	animdesc_t field_4;
} turkeyanims_t;

extern byte scratch_mem1[8010];
extern byte *scratch_mem2;

extern rect_t room_bounds_rect;

extern byte last_object_hint;
extern byte object_hint;
extern byte command_hint;
extern byte last_command_hint;

extern uint16 next_protozorqs_ticks;
extern uint16 next_vorts_ticks;
extern uint16 next_vorts_cmd;
extern uint16 next_turkey_ticks;
extern uint16 next_turkey_cmd;

#define MAX_SPRITES 16

extern byte *sprites_list[MAX_SPRITES];

#define MAX_DOORS 5

extern byte *doors_list[MAX_DOORS];

extern byte zone_palette;

extern spot_t *zone_spots;
extern spot_t *zone_spots_end;
extern spot_t *zone_spots_cur;

extern vortanims_t vortsanim_list[];
extern vortanims_t *vortanims_ptr;

extern turkeyanims_t turkeyanim_list[];
extern turkeyanims_t *turkeyanims_ptr;
extern pers_t *aspirant_ptr;
extern spot_t *aspirant_spot;
extern spot_t *found_spot;
extern byte **spot_sprite;

extern byte *lutin_mem;

extern byte skip_zone_transition;

extern byte in_de_profundis;

extern byte zone_name;
extern byte room_hint_bar_width;
extern byte zone_spr_index;
extern byte zone_obj_count;
extern byte room_hint_bar_coords_x;
extern byte room_hint_bar_coords_y;

extern uint16 drops_cleanup_time;

extern const byte patrol_route[];
extern const byte *timed_seq_ptr;

typedef struct thewalldoor_t {
	byte   height;
	byte   width;
	unsigned int    pitch;
	unsigned int    offs;
	byte   *pixels;
} thewalldoor_t;

extern thewalldoor_t the_wall_doors[2];

int IsInRect(byte x, byte y, rect_t *rect);
int IsCursorInRect(rect_t *rect);
void SelectSpotCursor(void);

void CheckHotspots(byte m, byte v);

void AnimateSpot(const animdesc_t *info);
byte *LoadPuzzlToScratch(byte index);

void DrawObjectHint(void);
void ShowObjectHint(byte *target);
void DrawCommandHint(void);
void ShowCommandHint(byte *target);

void DrawCharacterSprite(byte spridx, byte x, byte y, byte *target);
char DrawZoneAniSprite(rect_t *rect, unsigned int index, byte *target);

void RedrawHintsAndCursor(void);
void DrawHintsAndCursor(void);

void DrawTheWallDoors(void);
void MergeSpritesData(byte *target, unsigned int pitch, byte *source, unsigned int w, unsigned int h);
void MergeSpritesDataFlip(byte *target, unsigned int pitch, byte *source, unsigned int w, unsigned int h);

void RefreshSpritesData(void);
void BlitSpritesToBackBuffer(void);
byte *BackupSpotImage(spot_t *spot, byte **spotback, byte *buffer);
void BackupSpotsImages(void);

void SelectPalette(void);
void SelectSpecificPalette(byte index);

byte FindSpotByFlags(byte mask, byte value);
byte SelectPerson(byte offset);

void FindPerson(void);

void BeforeChangeZone(byte index);
void DrawRoomItemsIndicator(void);
void DrawRoomStaticObject(byte *aptr, byte *rx, byte *ry, byte *rw, byte *rh);
void DrawRoomStatics(void);
void RedrawRoomStatics(byte index, byte y_step);
void DrawPersons(void);
void RefreshZone(void);
void ChangeZone(byte index);

void DrawSpots(byte *target);
void AnimateSpots(byte *target);

byte FindInitialSpot(void);
void AnimRoomDoorOpen(byte index);
void AnimRoomDoorClose(byte index);

unsigned int GetPuzzlSprite(byte index, byte x, byte y, unsigned int *w, unsigned int *h, unsigned int *ofs);

void BounceCurrentItem(byte flags, byte y);

void BackupScreenOfSpecialRoom(void);
void RestoreScreenOfSpecialRoom(void);

void TheWallPhase3_DoorOpen1(void);
void TheWallPhase0_DoorOpen2(void);
void TheWallPhase1_DoorClose1(void);
void TheWallPhase2_DoorClose2(void);

void PrepareAspirant(void);
void PrepareVorts(void);
void PrepareTurkey(void);

void UpdateProtozorqs(void);
void CheckGameTimeLimit(void);
void CleanupDroppedItems(void);

void ResetAllPersons(void);

#endif
