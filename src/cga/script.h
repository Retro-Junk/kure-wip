#ifndef _SCRIPT_H_
#define _SCRIPT_H_

#include "common.h"
#include "room.h"

enum ScriptPools {
	ScrPool0_WordVars0,
	ScrPool1_WordVars1,
	ScrPool2_ByteVars,
	ScrPool3_CurrentItem,
	ScrPool4_ZoneSpots,
	ScrPool5_Persons,
	ScrPool6_Inventory,
	ScrPool7_Zapstiks,
	ScrPool8_CurrentPers,
	ScrPools_MAX
};

/*Byte-packed, members accessed from script code by hardcoded offsets*/
typedef struct script_byte_vars_t {
	unsigned char zone_index;       /*  0 */
	unsigned char zone_room;        /*  1 */
	unsigned char last_door;        /*  2 */
	unsigned char cur_spot_idx;     /*  3 */
	unsigned char the_wall_phase;   /*  4 */
	unsigned char prev_zone_index;  /*  5 */
	unsigned char bvar_06;          /*  6 */
	unsigned char bvar_07;          /*  7 */
	unsigned char bvar_08;          /*  8 */
	unsigned char bvar_09;          /*  9 */
	unsigned char bvar_0A;          /*  A */
	unsigned char bvar_0B;          /*  B */
	unsigned char zone_area;        /*  C */
	unsigned char dead_flag;        /*  D */
	volatile unsigned char timer_ticks; /*  E */
	unsigned char gauss_phase;      /*  F */

	unsigned char bvar_10;          /* 10 */
	unsigned char rand_value;       /* 11 */
	unsigned char load_flag;        /* 12 */
	unsigned char spot_m;           /* 13 */
	unsigned char spot_v;           /* 14 */
	unsigned char bvar_15;          /* 15 */
	unsigned char bvar_16;          /* 16 */
	unsigned char bvar_17;          /* 17 */
	unsigned char bvar_18;          /* 18 */
	unsigned char bvar_19;          /* 19 */
	unsigned char bvar_1A;          /* 1A */
	unsigned char bvar_1B;          /* 1B */
	unsigned char bvar_1C;          /* 1C */
	unsigned char bvar_1D;          /* 1D */
	unsigned char bvar_1E;          /* 1E */
	unsigned char bvar_1F;          /* 1F */

	unsigned char cur_pers;         /* 20 */
	unsigned char used_commands;    /* 21 */
	unsigned char tries_left;       /* 22 */
	unsigned char inv_item_index;   /* 23 */
	unsigned char bvar_24;          /* 24 */
	unsigned char bvar_25;          /* 25 */
	unsigned char bvar_26;          /* 26 */
	unsigned char bvar_27;          /* 27 */
	unsigned char bvar_28;          /* 28 */
	unsigned char bvar_29;          /* 29 */
	unsigned char bvar_2A;          /* 2A */
	unsigned char hands;            /* 2B */
	unsigned char check_used_commands; /* 2C */
	unsigned char bvar_2D;          /* 2D */
	unsigned char palette_index;    /* 2E */
	unsigned char bvar_2F;          /* 2F */

	unsigned char bvar_30;          /* 30 */
	unsigned char zapstiks_owned;   /* 31 */
	unsigned char bvar_32;          /* 32 */
	unsigned char bvar_33;          /* 33 */
	unsigned char bvar_34;          /* 34 */
	unsigned char skulls_submitted; /* 35 */
	unsigned char bvar_36;          /* 36 */
	unsigned char bvar_37;          /* 37 */
	unsigned char zone_area_copy;   /* 38 */
	unsigned char aspirant_flags;   /* 39 */
	unsigned char aspirant_pers_ofs;/* 3A */
	unsigned char steals_count;     /* 3B */
	unsigned char fight_status;     /* 3C */
	unsigned char extreme_violence; /* 3D */
	unsigned char trade_accepted;   /* 3E */
	unsigned char bvar_3F;          /* 3F */

	unsigned char bvar_40;          /* 40 */
	unsigned char bvar_41;          /* 41 */
	unsigned char bvar_42;          /* 42 */
	unsigned char bvar_43;          /* 43 */
	unsigned char dirty_rect_kind;  /* 44 */
	unsigned char bvar_45;          /* 45 */
	unsigned char bvar_46;          /* 46 */
	unsigned char game_paused;      /* 47 */
	unsigned char skull_trader_status;/* 48 */
	unsigned char cur_spot_flags;   /* 49 */
	unsigned char bvar_4A;          /* 4A */
	unsigned char bvar_4B;          /* 4B */
	unsigned char bvar_4C;          /* 4C */
	unsigned char bvar_4D;          /* 4D */
	unsigned char bvar_4E;          /* 4E */
	unsigned char bvar_4F;          /* 4F */

	unsigned char bvar_50;          /* 50 */
	unsigned char bvar_51;          /* 51 */
	unsigned char bvar_52;          /* 52 */
	unsigned char bvar_53;          /* 53 */
	unsigned char bvar_54;          /* 54 */
	unsigned char bvar_55;          /* 55 */
	unsigned char bvar_56;          /* 56 */
	unsigned char need_draw_spots;  /* 57 */
	unsigned char bvar_58;          /* 58 */
	unsigned char bvar_59;          /* 59 */
	unsigned char psy_energy;       /* 5A */
	unsigned char bvar_5B;          /* 5B */
	unsigned char bvar_5C;          /* 5C */
	unsigned char bvar_5D;          /* 5D */
	unsigned char bvar_5E;          /* 5E */
	unsigned char bvar_5F;          /* 5F */

	unsigned char bvar_60;          /* 60 */
	unsigned char bvar_61;          /* 61 */
	unsigned char bvar_62;          /* 62 */
	unsigned char bvar_63;          /* 63 */
	unsigned char bvar_64;          /* 64 */
	unsigned char bvar_65;          /* 65 */
	unsigned char bvar_66;          /* 66 */
	unsigned char bvar_67;          /* 67 */
	unsigned char zapstik_stolen;   /* 68 */
	unsigned char bvar_69;          /* 69 */
	unsigned char bvar_6A;          /* 6A */
	unsigned char bvar_6B;          /* 6B */
	unsigned char bvar_6C;          /* 6C */
	unsigned char bvar_6D[4];       /* 6D */
} script_byte_vars_t;

/*2-byte long vars, in BIG-endian order*/
typedef struct script_word_vars_t {
	unsigned short psi_cmds[6];         /*  0 */
	unsigned short wvar_0C;             /*  C */
	unsigned short wvar_0E;             /*  E */
	unsigned short timer_ticks2;        /* 10 */
	unsigned short zone_obj_cmds[15 * 5];   /* 12 */
	unsigned short next_aspirant_cmd;   /* A8 */
	unsigned short wvar_AA;             /* AA */
	unsigned short wvar_AC;             /* AC */
	unsigned short wvar_AE;             /* AE */
	unsigned short wvar_B0;             /* B0 */
	unsigned short wvar_B2;             /* B2 */
	unsigned short wvar_B4;             /* B4 */
	unsigned short next_protozorqs_cmd; /* B6 */
	unsigned short wvar_B8;             /* B8 */
} script_word_vars_t;

extern void *script_vars[ScrPools_MAX];
extern script_word_vars_t script_word_vars;
extern script_byte_vars_t script_byte_vars;


/*Don't trade this item*/
#define ITEMFLG_DONTWANT 1
#define ITEMFLG_04 0x04
#define ITEMFLG_08 0x08
/*Skull Trader's item*/
#define ITEMFLG_TRADER 0x10
/*Aspirant's item*/
#define ITEMFLG_ASPIR 0x20
/*In a room?*/
#define ITEMFLG_ROOM 0x40
/*In pocket?*/
#define ITEMFLG_OWNED 0x80

/*TODO: manipulated from script, do not change*/
typedef struct item_t {
	unsigned char flags;
	unsigned char area;		/*item location*/
	unsigned char sprite;   /*item sprite index*/
	unsigned char name;     /*item name index (relative)*/
	unsigned short command; /*TODO: warning! in native format, check if never accessed from scripts*/
} item_t;

#define MAX_INV_ITEMS 63
extern item_t inventory_items[MAX_INV_ITEMS];

#define PERS_MAX 41
extern pers_t pers_list[PERS_MAX];

extern unsigned char *script_stack[5 * 2];
extern unsigned char **script_stack_ptr;

extern pers_t *vort_ptr;

#define SPECIAL_COMMANDS_MAX 20
extern unsigned short menu_commands_12[SPECIAL_COMMANDS_MAX];
extern unsigned short menu_commands_22[SPECIAL_COMMANDS_MAX];
extern unsigned short menu_commands_24[SPECIAL_COMMANDS_MAX];
extern unsigned short menu_commands_23[SPECIAL_COMMANDS_MAX];

extern unsigned short fight_pers_ofs;

extern unsigned char wait_delta;

extern unsigned char rand_seed;
unsigned char Rand(void);
unsigned short RandW(void);

extern unsigned short the_command;

unsigned char *GetScriptSubroutine(unsigned int index);

unsigned int RunCommand(void);
unsigned int RunCommandKeepSp(void);

unsigned short Swap16(unsigned short x);

#endif
