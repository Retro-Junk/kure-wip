#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#ifdef __WATCOMC__
#include <i86.h>
#endif
#include <io.h>
#include <fcntl.h>
#include <assert.h>
#include <setjmp.h>

#include "common.h"
#include "decompr.h"
#include "cga.h"
#include "ega.h"
#include "anim.h"
#include "cursor.h"
#include "input.h"
#include "timer.h"
#include "portrait.h"
#include "room.h"
#include "savegame.h"
#include "resdata.h"
#include "script.h"
#include "print.h"
#include "dialog.h"
#include "menu.h"
#include "ifgm.h"

uint16 cpu_speed_delay;

/*
Prompt user to insert disk #2 to any drive
*/
void AskDisk2(void) {
	the_command = 0x918E;
	ResetInput();
	RunCommand();
}

/*
Show game exit confirmation dialog and get user's input
*/
int AskQuitGame(void) {
	int quit = 0;
#ifdef VERSION_USA
	byte *msg = SeekToString(desci_data, 411);	/*DO YOU WANT TO QUIT ? (Y OR N).*/
	char_draw_max_width = 32;
	draw_x = 1;
	draw_y = 188;
	CGA_DrawTextBox(msg, frontbuffer);
	for (;;) {
		byte c = GetKeyScan();
		if (c == 0x15) {	/* Y */
			quit = 1;
			break;
		}
		if (c == 0x31) {	/* N */
			quit = 0;
			break;
		}
	}
	CGA_CopyScreenBlock(backbuffer, char_draw_max_width + 2, char_draw_coords_y - draw_y + 8, frontbuffer, CGA_CalcXY_p(draw_x, draw_y));
#endif
	/*EU version comes without requited text string*/
	return quit;
}

void SaveToFile(char *filename, void *data, unsigned int size) {
	FILE *f = fopen(filename, "wb");
	fwrite(data, 1, size, f);
	fclose(f);
}

int LoadSplash(const char *filename) {
	if (!LoadFile(filename, backbuffer))
		return 0;
	return 1;
}

unsigned int BenchmarkCpu(void) {
	byte t;
	unsigned int cycles = 0;
	for (t = script_byte_vars.timer_ticks; t == script_byte_vars.timer_ticks;) ;
	for (t = script_byte_vars.timer_ticks; t == script_byte_vars.timer_ticks;) cycles++;
	return cycles;
}

void Randomize(void) {
	union REGS reg;

	reg.h.ah = 0;
#ifdef __386__
	int386(0x1A, &reg, &reg);
#else
	int86(0x1A, &reg, &reg);
#endif
	rand_seed = reg.h.dl;
	Rand();
}

void TRAP() {
	PromptWait();
	for (;;) ;
}

/* Main Game Loop */
void GameLoop(void) {
#if 0
	for (;;) {
		/* Update/check live things */
		UpdateProtozorqs();
		CheckGameTimeLimit();
		CleanupDroppedItems();

		/* Get player input */
		PollInput();

		UndrawCursor();
		AnimateSpots();
		UpdateCursor();

		the_command = 0;
		if (IsCursorInRect(&room_bounds_rect)) {
			SelectCursor(CURSOR_TARGET);
			command_hint = 100;
			SelectSpotCursor();
		} else {
			SelectCursor(CURSOR_FINGER);
			object_hint = 117;
			CheckMenuCommandHover();
		}

		if (object_hint != last_object_hint || object_hint_changed) {
			object_hint_changed = object_hint != last_object_hint;
			DrawObjectHint();
		}

		if (command_hint != last_command_hint || command_hint_changed) {
			command_hint_changed = command_hint != last_command_hint;
			DrawCommandHint();
		}

		RedrawHintsAndCursor();

		if (!buttons || !the_command) {
			/*Pending / AI commands*/

			if (script_byte_vars.check_used_commands < script_byte_vars.used_commands) {
				the_command = Swap16(script_word_vars.next_aspirant_cmd);
				if (the_command)
					goto process;
			}

			if (script_byte_vars.bvar_45)
				continue;

			the_command = Swap16(script_word_vars.next_protozorqs_cmd);
			if (the_command)
				goto process;

			if (Swap16(next_vorts_ticks) < script_word_vars.timer_ticks2) { /*TODO: is this ok? ticks2 is BE, next_vorts_ticks is LE*/
				the_command = next_vorts_cmd;
				if (the_command)
					goto process;
			}

			if (Swap16(next_turkey_ticks) < script_word_vars.timer_ticks2) { /*TODO: is this ok? ticks2 is BE, next_turkey_ticks is LE*/
				the_command = next_turkey_cmd;
				if (the_command)
					goto process;
			}

			continue;

process:;
			UpdateUndrawCursor();
			RefreshSpritesData();
			RunCommand();
			BlitSpritesToBackBuffer();
			/*TODO*/
			ProcessInput();
			last_command_hint = last_object_hint = 0;
		} else {
			/*Player action*/

			UpdateUndrawCursor();
			RefreshSpritesData();
			RunCommandKeepSp();
			script_byte_vars.used_commands++;
			if (script_byte_vars.dead_flag) {
				if (--script_byte_vars.tries_left == 0)
					ResetAllPersons();
			}
			BlitSpritesToBackBuffer();
			/*TODO*/
			ProcessInput();
			last_command_hint = last_object_hint = 0;
		}
	}
#endif
}


void ExitGame(char *msg) {
	EGA_SwitchToTextMode();
	UninitTimer();
	printf("Exit %s\n", msg);
	exit(0);
}

jmp_buf restart_jmp;

#ifdef DEBUG_ENDING
extern TheEnd(void);
#endif

void main(void) {

	EGA_SwitchToGraphicsMode();

	/*TODO DetectCPU();*/

	/* Initialize audio engine */
	IFGM_Init();

	/* Install timer callback */
	InitTimer();

	/* Load script and other static resources */
	/* Those are normally embedded in the executable, but here we load the extracted ones*/
	if (!LoadStaticData())
		ExitGame("!LoadStatic");

	/* Load title screen */
	if (!LoadSplash("PRESEGA.EGA"))
		ExitGame("!Load PRES");

	if (ifgm_loaded) {
		/*TODO*/
	}

	/* Show the title screen */
	EGA_BackBufferToRealFull();
	EGA_Flip();

	if (ifgm_loaded) {
		/*TODO*/
	}

	/* Load text resources */
	if (!LoadTextData())
		ExitGame("!LoadText");

	/* Detect/Initialize input device */
	InitInput();

	/* Load room sprites */
	if (!LoadPuzzlData())
		ExitGame("!LoadPuzzl");

	/* Load main background screen */
	while (!LoadFond()) {
		AskDisk2();
	}

	/* Display it */
	EGA_BackToRealBothPages();

	/* Load graphics resources */
	while (!LoadSpritesData()) {
		AskDisk2();
	}

	/*TODO: is this neccessary?*/
	EGA_BackToRealBothPages();

	/* Create clean game state snapshot */
	SaveRestartGame();

	if (ifgm_loaded) {
		/*TODO*/
	}

	/*restart game from here*/
restart:;
	setjmp(restart_jmp);

	Randomize();

	/* Set start zone */
	script_byte_vars.zone_index = 7;

	ega_busy &= ~1;

	/* Begin the game */
	script_byte_vars.game_paused = 0;

#ifdef DEBUG_SCRIPT
	unlink(DEBUG_SCRIPT_LOG);
#endif

#ifdef DEBUG_SKIP_INTRO
	/*bypass characters introduction*/
	script_byte_vars.load_flag = DEBUG_SKIP_INTRO;
#endif

	/* Play introduction sequence and initialize game */
	the_command = 0xC001;
	RunCommand();

for(;;);

	/* Sync graphics */
	BlitSpritesToBackBuffer();

	/* Initialize cursor backup */
	ProcessInput();

#ifdef DEBUG_ENDING
	script_byte_vars.game_paused = 5;
	TheEnd();
	for (;;) ;
#endif

	/* Main game loop */
	GameLoop();

	/*TODO: the following code is never executed since GameLoop is infinite (or the whole game is restarted) */

	/* Release hardware */
	UninitInput();

	ExitGame("OK");
}
