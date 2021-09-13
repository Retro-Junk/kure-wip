#include <io.h>
#include <fcntl.h>
#include "common.h"
#include "savegame.h"
#include "resdata.h"
#include "script.h"
#include "cga.h"
#include "room.h"
#include "dialog.h"

void SaveRestartGame(void)
{
	/*TODO*/
}

void RestartGame(void)
{
	/*
	while(!LoadFile("CLEAR.BIN", save_start))
		AskDisk2();
	*/

	script_byte_vars.cur_spot_flags = 0xFF;
	script_byte_vars.load_flag = 2;
	/*Restart();*/

	/*TODO*/
}

#define CGA_SAVE_BEG_OFS 0x751E
#define CGA_SAVE_END_OFS 0x9D5D

#define CGA_SAVE_WORD_VARS_OFS 0x7844
#define CGA_SAVE_BYTE_VARS_OFS 0x79B6
#define CGA_SAVE_INVENTORY_OFS 0x75E5
#define CGA_SAVE_ZONES_OFS     0x7A27
#define CGA_SAVE_PERS_OFS      0x7765
#define CGA_SAVE_STACK_OFS     0x7562
#define CGA_SAVE_SPRITES_OFS   0x5264
#define CGA_SAVE_SPRLIST_OFS   0x7530
#define CGA_SAVE_SPRBUFF_OFS   0x4C68
#define CGA_SAVE_ARPLA_OFS     0x26F6
#define CGA_SAVE_SCRSTACK_OFS  0x7562
#define CGA_SAVE_TEMPL_OFS     0x182C
#define CGA_SAVE_VORTANIMS_OFS 0xA609
#define CGA_SAVE_RECS7_OFS     0xA74E
#define CGA_SAVE_TIMEDSEQ_OFS  0xA7C0

#define SAVEADDR(value, base, nativesize, origsize, origbase)	\
	((value) ? LE16(((((unsigned char*)(value)) - (unsigned char*)(base)) / nativesize) * origsize + origbase) : 0)

#define LOADADDR(value, base, nativesize, origsize, origbase)	\
	((value) ? ((((LE16(value)) - (origbase)) / origsize) * nativesize + (unsigned char*)base) : 0)

#define WRITE(buffer, size) \
	wlen = write(f, buffer, size); if(wlen != size) goto error;

#define READ(buffer, size) \
	rlen = read(f, buffer, size); if(rlen != size) goto error;

int LoadScena(void)
{
	int f;
	int rlen;
	unsigned short zero = 0;
	unsigned char *p;
	int i;

	script_byte_vars.game_paused = 1;


	f = open("SCENAx.BIN", O_RDONLY | O_BINARY);
	if(f == -1)
	{
		script_byte_vars.game_paused = 0;
		return 1;	/*error*/
	}
	/*
	Save format:
	  vars memory (751E-9D5D)
	  frontbuffer (0x3FFF bytes)
      backbuffer  (0x3FFF bytes)
	*/

	#define BYTES(buffer, size) READ(buffer, size)
	#define UBYTE(variable) { unsigned char temp_v; READ(&temp_v, 1); variable = temp_v; }
	#define SBYTE(variable) { signed char temp_v; READ(&temp_v, 1); variable = temp_v; }
	#define USHORT(variable) { unsigned short temp_v; READ(&temp_v, 2); variable = temp_v; }
	#define SSHORT(variable) { signed short temp_v; READ(&temp_v, 2); variable = temp_v; }
	#define POINTER(variable, base, nativesize, origsize, origbase) \
		{ signed short temp_v; READ(&temp_v, 2); variable = LOADADDR(temp_v, base, nativesize, origsize, origbase); }

	/*script_vars pointers*/
	POINTER(script_vars[ScrPool0_WordVars0], &script_word_vars, 2, 2, CGA_SAVE_WORD_VARS_OFS);
	POINTER(script_vars[ScrPool1_WordVars1], &script_word_vars, 2, 2, CGA_SAVE_WORD_VARS_OFS);
	POINTER(script_vars[ScrPool2_ByteVars], &script_byte_vars, 1, 1, CGA_SAVE_BYTE_VARS_OFS);
	POINTER(script_vars[ScrPool3_CurrentItem], inventory_items, sizeof(item_t), sizeof(item_t), CGA_SAVE_INVENTORY_OFS);
	POINTER(script_vars[ScrPool4_ZoneSpots], zones_data, 1, 1, CGA_SAVE_ZONES_OFS);
	POINTER(script_vars[ScrPool5_Persons], pers_list, 1, 1, CGA_SAVE_PERS_OFS);
	POINTER(script_vars[ScrPool6_Inventory], inventory_items, sizeof(item_t), sizeof(item_t), CGA_SAVE_INVENTORY_OFS);
	POINTER(script_vars[ScrPool7_Inventory38], inventory_items, sizeof(item_t), sizeof(item_t), CGA_SAVE_INVENTORY_OFS);
	POINTER(script_vars[ScrPool8_CurrentPers], pers_list, 1, 1, CGA_SAVE_PERS_OFS);

	/* sprites_list */
	for(i = 0;i < MAX_SPRITES;i++)
	{
		POINTER(sprites_list[i], scratch_mem1, 1, 1, CGA_SAVE_SPRITES_OFS);
	}

	/* doors list */
	for(i = 0;i < MAX_DOORS;i++)
	{
		POINTER(doors_list[i], arpla_data, 1, 1, CGA_SAVE_ARPLA_OFS);
	}

	/* zone_spots */
	POINTER((unsigned char*)zone_spots, zones_data, 1, 1, CGA_SAVE_ZONES_OFS);

	/* zone_spots_end */
	POINTER((unsigned char*)zone_spots_end, zones_data, 1, 1, CGA_SAVE_ZONES_OFS);

	/* zone_spots_cur */
	POINTER((unsigned char*)zone_spots_cur, zones_data, 1, 1, CGA_SAVE_ZONES_OFS);

	/* script_stack_ptr */
	/*TODO: FIX ME: original stack works in reverse order (from higher address to lower)*/
	POINTER((unsigned char*)script_stack_ptr, script_stack, 1, 1, CGA_SAVE_SCRSTACK_OFS);

	/* script_stack */
	/*TODO: FIX ME: original stack works in reverse order (from higher address to lower)*/
	for(i = 0;i < 5*2;i++)
	{
		POINTER(script_stack[i], templ_data, 1, 1, CGA_SAVE_TEMPL_OFS);
	}

	/* padding */
	USHORT(zero);

	/* pers_vort_ptr */
	POINTER((unsigned char*)pers_vort_ptr, pers_list, 1, 1, CGA_SAVE_PERS_OFS);

	/* vortanims_ptr */
	POINTER((unsigned char*)vortanims_ptr, vortsanim_list, 1, 1, CGA_SAVE_VORTANIMS_OFS);

	/* rec7_ptr */
	POINTER((unsigned char*)rec7_ptr, recs7_list, 1, 1, CGA_SAVE_RECS7_OFS);

	/* pers_ptr */
	POINTER((unsigned char*)pers_ptr, pers_list, 1, 1, CGA_SAVE_PERS_OFS);

	/* spot_ptr */
	POINTER((unsigned char*)spot_ptr, zones_data, 1, 1, CGA_SAVE_ZONES_OFS);

	/* found_spot */
	POINTER((unsigned char*)found_spot, zones_data, 1, 1, CGA_SAVE_ZONES_OFS);

	/* spot_sprite */
	POINTER((unsigned char*)spot_sprite, sprites_list, sizeof(sprites_list[0]), 2, CGA_SAVE_SPRLIST_OFS);

	/* timed_seq_ptr */
	POINTER(timed_seq_ptr, timed_seq, 1, 1, CGA_SAVE_TIMEDSEQ_OFS);

	/* keep_sp */
	/* TODO: how to save it? but it's probably useless anyway */
	USHORT(zero);

	/* unused ptr to script code */
	p = templ_data;
	POINTER(p, templ_data, 1, 1, CGA_SAVE_TEMPL_OFS);

	/* padding */
	UBYTE(zero);

	/* the wall doors state */
	for(i = 0;i < 2;i++)
	{
		thewalldoor_t *door = &the_wall_doors[i];
		UBYTE(door->height);
		UBYTE(door->width);
		USHORT(door->pitch);
		USHORT(door->offs);
		POINTER(door->pixels, sprit_load_buffer, 1, 1, CGA_SAVE_SPRBUFF_OFS);
	}

	/* wait_delta */
	UBYTE(wait_delta);

	/* padding */
	UBYTE(zero);

	/* dirty_rects */
	for(i = 0;i < MAX_DIRTY_RECT;i++)
	{
		dirty_rect_t *dr = &dirty_rects[i];
		UBYTE(dr->kind);
		USHORT(dr->offs);
		UBYTE(dr->height);
		UBYTE(dr->width);
		UBYTE(dr->y);
		UBYTE(dr->x);
	}

	/* inventory_items */
	for(i = 0;i < MAX_INV_ITEMS;i++)
	{
		/*TODO: properly serialize this*/
		BYTES(&inventory_items[i], sizeof(item_t));
	}

	/* room_hint_bar_coords_y */
	UBYTE(room_hint_bar_coords_y);

	/* room_hint_bar_coords_x */
	UBYTE(room_hint_bar_coords_x);

	/* padding */
	USHORT(zero);

	/* fight_pers_ofs */
	/* NB! raw offset */
	USHORT(fight_pers_ofs);

	/* pers_list */
	for(i = 0;i < PERS_MAX;i++)
	{
		/*TODO: properly serialize this*/
		BYTES(&pers_list[i], sizeof(pers_t));
	}

	/* inv_update_time */
	USHORT(inv_update_time);

	/* room_bounds_rect */
	/*TODO: properly serialize this*/
	BYTES(&room_bounds_rect, sizeof(rect_t));

	/* last_object_hint */
	UBYTE(last_object_hint);

	/* object_hint */
	UBYTE(object_hint);

	/* command_hint */
	UBYTE(command_hint);

	/* zone_name */
	UBYTE(zone_name);

	/* room_hint_bar_width */
	UBYTE(room_hint_bar_width);

	/* last_command_hint */
	UBYTE(last_command_hint);

	/* zone_spr_index */
	UBYTE(zone_spr_index);

	/* zone_obj_count */
	UBYTE(zone_obj_count);

	/* padding */
	USHORT(zero);

	/* padding */
	UBYTE(zero);

	/* in_de_profundis */
	UBYTE(in_de_profundis);

	/* script_word_vars */
	BYTES(&script_word_vars, sizeof(script_word_vars));

	/* menu_commands_12 */
	BYTES(menu_commands_12, sizeof(menu_commands_12));

	/* menu_commands_22 */
	BYTES(menu_commands_22, sizeof(menu_commands_22));

	/* menu_commands_24 */
	BYTES(menu_commands_24, sizeof(menu_commands_24));

	/* menu_commands_23 */
	BYTES(menu_commands_23, sizeof(menu_commands_23));

	/* next_command3 */
	USHORT(next_command3);

	/* next_ticks3 */
	USHORT(next_ticks3);

	/* next_command4 */
	USHORT(next_command4);

	/* next_ticks4 */
	USHORT(next_ticks4);

	/* next_ticks2 */
	USHORT(next_ticks2);

	/* padding */
	for(i = 0;i < 7;i++) USHORT(zero);

	/* script_byte_vars */
	BYTES(&script_byte_vars, sizeof(script_byte_vars));

	/* zones_data */
	BYTES(zones_data, RES_ZONES_MAX);

	/* screen data */
	BYTES(backbuffer, 0x3FFF);

	CGA_BackBufferToRealFull();
	SelectPalette();

	BYTES(backbuffer, 0x3FFF);

	#undef BYTES
	#undef UBYTE
	#undef SBYTE
	#undef USHORT
	#undef SSHORT
	#undef POINTER

	/*re-initialize sprites list*/
	BackupSpotsImages();

	close(f);
	script_byte_vars.game_paused = 0;
	return 0;

error:;
	close(f);
	script_byte_vars.game_paused = 0;
	return 1;
}

int SaveScena(void)
{
	int f;
	int wlen;
	unsigned short zero = 0;
	unsigned char *p;
	int i;

	script_byte_vars.game_paused = 1;
	BlitSpritesToBackBuffer();

	f = open("SCENAx.BIN", O_CREAT | O_WRONLY | O_BINARY);
	if(f == -1)
	{
		script_byte_vars.game_paused = 0;
		return 1;	/*error*/
	}

	#define BYTES(buffer, size) WRITE(buffer, size)
	#define UBYTE(variable) { unsigned char temp_v = variable; WRITE(&temp_v, 1); }
	#define SBYTE(variable) { signed char temp_v = variable; WRITE(&temp_v, 1); }
	#define USHORT(variable) { unsigned short temp_v = variable; WRITE(&temp_v, 2); }
	#define SSHORT(variable) { signed short temp_v = variable; WRITE(&temp_v, 2); }
	#define POINTER(variable, base, nativesize, origsize, origbase) \
		{ signed short temp_v = SAVEADDR(variable, base, nativesize, origsize, origbase); WRITE(&temp_v, 2); }

	/*script_vars pointers*/
	POINTER(script_vars[ScrPool0_WordVars0], &script_word_vars, 2, 2, CGA_SAVE_WORD_VARS_OFS);
	POINTER(script_vars[ScrPool1_WordVars1], &script_word_vars, 2, 2, CGA_SAVE_WORD_VARS_OFS);
	POINTER(script_vars[ScrPool2_ByteVars], &script_byte_vars, 1, 1, CGA_SAVE_BYTE_VARS_OFS);
	POINTER(script_vars[ScrPool3_CurrentItem], inventory_items, sizeof(item_t), sizeof(item_t), CGA_SAVE_INVENTORY_OFS);
	POINTER(script_vars[ScrPool4_ZoneSpots], zones_data, 1, 1, CGA_SAVE_ZONES_OFS);
	POINTER(script_vars[ScrPool5_Persons], pers_list, 1, 1, CGA_SAVE_PERS_OFS);
	POINTER(script_vars[ScrPool6_Inventory], inventory_items, sizeof(item_t), sizeof(item_t), CGA_SAVE_INVENTORY_OFS);
	POINTER(script_vars[ScrPool7_Inventory38], inventory_items, sizeof(item_t), sizeof(item_t), CGA_SAVE_INVENTORY_OFS);
	POINTER(script_vars[ScrPool8_CurrentPers], pers_list, 1, 1, CGA_SAVE_PERS_OFS);

	/* sprites_list */
	for(i = 0;i < MAX_SPRITES;i++)
	{
		POINTER(sprites_list[i], scratch_mem1, 1, 1, CGA_SAVE_SPRITES_OFS);
	}

	/* doors list */
	for(i = 0;i < MAX_DOORS;i++)
	{
		POINTER(doors_list[i], arpla_data, 1, 1, CGA_SAVE_ARPLA_OFS);
	}

	/* zone_spots */
	POINTER(zone_spots, zones_data, 1, 1, CGA_SAVE_ZONES_OFS);

	/* zone_spots_end */
	POINTER(zone_spots_end, zones_data, 1, 1, CGA_SAVE_ZONES_OFS);

	/* zone_spots_cur */
	POINTER(zone_spots_cur, zones_data, 1, 1, CGA_SAVE_ZONES_OFS);

	/* script_stack_ptr */
	/*TODO: FIX ME: original stack works in reverse order (from higher address to lower)*/
	POINTER(script_stack_ptr, script_stack, 1, 1, CGA_SAVE_SCRSTACK_OFS);

	/* script_stack */
	/*TODO: FIX ME: original stack works in reverse order (from higher address to lower)*/
	for(i = 0;i < 5*2;i++)
	{
		POINTER(script_stack[i], templ_data, 1, 1, CGA_SAVE_TEMPL_OFS);
	}

	/* padding */
	USHORT(zero);

	/* pers_vort_ptr */
	POINTER(pers_vort_ptr, pers_list, 1, 1, CGA_SAVE_PERS_OFS);

	/* vortanims_ptr */
	POINTER(vortanims_ptr, vortsanim_list, 1, 1, CGA_SAVE_VORTANIMS_OFS);

	/* rec7_ptr */
	POINTER(rec7_ptr, recs7_list, 1, 1, CGA_SAVE_RECS7_OFS);

	/* pers_ptr */
	POINTER(pers_ptr, pers_list, 1, 1, CGA_SAVE_PERS_OFS);

	/* spot_ptr */
	POINTER(spot_ptr, zones_data, 1, 1, CGA_SAVE_ZONES_OFS);

	/* found_spot */
	POINTER(found_spot, zones_data, 1, 1, CGA_SAVE_ZONES_OFS);

	/* spot_sprite */
	POINTER(spot_sprite, sprites_list, sizeof(sprites_list[0]), 2, CGA_SAVE_SPRLIST_OFS);

	/* timed_seq_ptr */
	POINTER(timed_seq_ptr, timed_seq, 1, 1, CGA_SAVE_TIMEDSEQ_OFS);

	/* keep_sp */
	/* TODO: how to save it? but it's probably useless anyway */
	USHORT(zero);

	/* unused ptr to script code */
	p = templ_data;
	POINTER(p, templ_data, 1, 1, CGA_SAVE_TEMPL_OFS);

	/* padding */
	UBYTE(zero);

	/* the wall doors state */
	for(i = 0;i < 2;i++)
	{
		thewalldoor_t *door = &the_wall_doors[i];
		UBYTE(door->height);
		UBYTE(door->width);
		USHORT(door->pitch);
		USHORT(door->offs);
		POINTER(door->pixels, sprit_load_buffer, 1, 1, CGA_SAVE_SPRBUFF_OFS);
	}

	/* wait_delta */
	UBYTE(wait_delta);

	/* padding */
	UBYTE(zero);

	/* dirty_rects */
	for(i = 0;i < MAX_DIRTY_RECT;i++)
	{
		dirty_rect_t *dr = &dirty_rects[i];
		UBYTE(dr->kind);
		USHORT(dr->offs);
		UBYTE(dr->height);
		UBYTE(dr->width);
		UBYTE(dr->y);
		UBYTE(dr->x);
	}

	/* inventory_items */
	for(i = 0;i < MAX_INV_ITEMS;i++)
	{
		/*TODO: properly serialize this*/
		BYTES(&inventory_items[i], sizeof(item_t));
	}

	/* room_hint_bar_coords_y */
	UBYTE(room_hint_bar_coords_y);

	/* room_hint_bar_coords_x */
	UBYTE(room_hint_bar_coords_x);

	/* padding */
	USHORT(zero);

	/* fight_pers_ofs */
	/* NB! raw offset */
	USHORT(fight_pers_ofs);

	/* pers_list */
	for(i = 0;i < PERS_MAX;i++)
	{
		/*TODO: properly serialize this*/
		BYTES(&pers_list[i], sizeof(pers_t));
	}

	/* inv_update_time */
	USHORT(inv_update_time);

	/* room_bounds_rect */
	/*TODO: properly serialize this*/
	BYTES(&room_bounds_rect, sizeof(rect_t));

	/* last_object_hint */
	UBYTE(last_object_hint);

	/* object_hint */
	UBYTE(object_hint);

	/* command_hint */
	UBYTE(command_hint);

	/* zone_name */
	UBYTE(zone_name);

	/* room_hint_bar_width */
	UBYTE(room_hint_bar_width);

	/* last_command_hint */
	UBYTE(last_command_hint);

	/* zone_spr_index */
	UBYTE(zone_spr_index);

	/* zone_obj_count */
	UBYTE(zone_obj_count);

	/* padding */
	USHORT(zero);

	/* padding */
	UBYTE(zero);

	/* in_de_profundis */
	UBYTE(in_de_profundis);

	/* script_word_vars */
	BYTES(&script_word_vars, sizeof(script_word_vars));

	/* menu_commands_12 */
	BYTES(menu_commands_12, sizeof(menu_commands_12));

	/* menu_commands_22 */
	BYTES(menu_commands_22, sizeof(menu_commands_22));

	/* menu_commands_24 */
	BYTES(menu_commands_24, sizeof(menu_commands_24));

	/* menu_commands_23 */
	BYTES(menu_commands_23, sizeof(menu_commands_23));

	/* next_command3 */
	USHORT(next_command3);

	/* next_ticks3 */
	USHORT(next_ticks3);

	/* next_command4 */
	USHORT(next_command4);

	/* next_ticks4 */
	USHORT(next_ticks4);

	/* next_ticks2 */
	USHORT(next_ticks2);

	/* padding */
	for(i = 0;i < 7;i++) USHORT(zero);

	/* script_byte_vars */
	BYTES(&script_byte_vars, sizeof(script_byte_vars));

	/* zones_data */
	BYTES(zones_data, RES_ZONES_MAX);

	/* screen data */
	BYTES(frontbuffer, 0x3FFF);
	BYTES(backbuffer, 0x3FFF);

	#undef BYTES
	#undef UBYTE
	#undef SBYTE
	#undef USHORT
	#undef SSHORT
	#undef POINTER

	close(f);
	script_byte_vars.game_paused = 0;
	return 0;

error:;
	close(f);
	script_byte_vars.game_paused = 0;
	return 1;
}
