#include <stdlib.h>
#include <dos.h>
#include <conio.h>
#include <bios.h>
#ifdef __386__
#include <i86.h>
#endif
#include "common.h"
#include "input.h"
#include "cursor.h"
#include "cga.h"
#include "timer.h"
#include "ifgm.h"

byte have_mouse = 0;
byte have_joystick = 0;
byte key_held;
volatile byte key_direction;
volatile byte key_code;

volatile byte keyboard_scan;
volatile byte keyboard_specials;
volatile byte keyboard_arrows;
volatile byte keyboard_buttons;

byte buttons_repeat = 0;
byte buttons;
byte right_button;
byte key_direction_old;
byte accell_countdown;
unsigned int accelleration = 1;

void (INTERRUPT *old_keyboard_isr)(void);

void PollDiscrete(void);

byte ReadKeyboardChar(void) {
	return (byte)getch();
}

void ClearKeyboard(void) {
	while (kbhit()) {
		getch();
	}
}

void INTERRUPT NullIsr(void) {
	/*nothing*/
}

/*Return currently held button's scancode*/
byte GetKeyScan(void) {
	byte scan = keyboard_scan;
	if (have_mouse) {
		if (bioskey(1)) {
			scan = (bioskey(0) >> 8) & 0xFF;
		}
	}
	return scan;
}

#ifdef VERSION_USA
extern int AskQuitGame(void);

void CheckExitRequest(void) {
	/*ESC pressed?*/
	if(GetKeyScan() == 1) {
		keyboard_scan = 0;
		if (AskQuitGame() != 0) {
			IFGM_Shutdown();
			UninitInput();
			UninitTimer();
			SwitchToTextMode();
			exit(-1);
		}
	}
}
#endif

void SetInputButtons(byte keys) {
#ifdef VERSION_USA
	CheckExitRequest();
#endif
	if (keys && buttons_repeat) {
		/*ignore buttons repeat*/
		buttons = 0;
		return;
	}
	if (keys & 2)
		right_button = ~0;
	if (keys & 1)
		right_button = 0;
	buttons = keys;
	buttons_repeat = keys;
}

byte PollMouseHw(unsigned int *curs_x, uint8 *curs_y) {
	union REGS reg;
#ifdef __386__
	reg.w.ax = 3;
	int386(0x33, &reg, &reg);
	*curs_x = reg.w.cx;
#else
	reg.x.ax = 3;
	int86(0x33, &reg, &reg);
	*curs_x = reg.x.cx;
#endif
	*curs_y = reg.h.dl;
	return reg.h.bl;    /*buttons*/
}

byte PollMouse(unsigned int *curs_x, uint8 *curs_y) {
	byte keys = PollMouseHw(curs_x, curs_y);
	if (kbhit())
		keys = 3;
	return keys;
}

byte ProcessDiscrete(void) {
	byte direction = key_direction;
	if (direction && direction == key_direction_old) {
		if (++accell_countdown == 10) {
			accelleration++;
			accell_countdown = 0;
		}
	} else {
		accelleration = 1;
		accell_countdown = 0;
	}
	key_direction_old = direction;

	if (direction & 8) {
		cursor_x += accelleration;
		if (cursor_x >= 304) /*TODO: >*/
			cursor_x = 304;
	}

	if (direction & 4) {
		cursor_x -= accelleration;
		if ((signed int)cursor_x < 0)	/*TODO: check if this is ok*/
			cursor_x = 0;
	}

	if (direction & 2) {
		cursor_y += accelleration;
		if (cursor_y >= 184) /*TODO: >*/
			cursor_y = 184;
	}

	if (direction & 1) {
		cursor_y -= accelleration;
		if ((int8)cursor_y < 0)		/*TODO: fixme, it will misbehave when cursor_y > 127*/
			cursor_y = 0;
	}

	return key_code;
}

/*
Poll input devices and update the cursor coordinates
*/
void PollInput(void) {
	byte keys;
	if (have_mouse) {
		keys = PollMouse(&cursor_x, &cursor_y);
	} else {
		PollDiscrete();
		keys = ProcessDiscrete();
	}
	SetInputButtons(keys);
}

void ProcessInput(void) {
	PollInput();
	DrawCursor();
}

void ResetInput(void) {
	keyboard_arrows = 0;
	keyboard_buttons = 0;
	buttons_repeat = 0;
}

/*
Poll Joystick and Keyboard directions/buttons
*/
void PollDiscrete(void) {
	byte joydpad = 0;
	byte joytrig = 0;
	if (have_joystick) {
		/*TODO: read joy's dpad state to joydpad*/
	}
	key_direction = keyboard_arrows | joydpad;

	if (have_joystick) {
		joytrig = (~(inportb(0x201) >> 4)) & 3;
	}
	key_code = keyboard_buttons | joytrig;
}

/*
Poll for action buttons only, ignore cursor movement
*/
void PollInputButtonsOnly(void) {
	byte butt = 0;
	if (have_mouse) {
		unsigned int tempx;
		uint8 tempy;
		butt = PollMouseHw(&tempx, &tempy);
		if (kbhit()) {
			getch();
			butt = 1;
		}
	}
	else {
		butt = keyboard_buttons;
		if (have_joystick) {
			butt |= (~(inportb(0x201) >> 4)) & 3;

		}
	}
	SetInputButtons(butt);
}

static struct {
	byte scan;
	byte action;
} keyboard_map[] = {
	{0x7C, 1},
	{0x7B, 2},
	{0x7A, 4},
	{0x79, 8},
	{0x77, 0x80 | (1<<4)},
	{0x7E,        (2<<4)},
	{0x7D, 0x80 | (1<<4)},
	{0x29, 1},		/*`*/
	{0x4A, 2},		/*-*/
	{0x2B, 4},		/*\*/
	{0x4E, 8},		/*+*/
	{0x48, 1},		/*Up*/
	{0x50, 2},		/*Down*/
	{0x4B, 4},		/*Left*/
	{0x4D, 8},		/*Right*/
	{0x39, 0x80 | (1<<4)},	/*Space*/
	{0x53, 0x40 | 0x10},	/*Del*/
	{0x1D, 0x40 | 1},	/*LCtrl*/
	{0x38, 0x40 | 2},	/*LAlt*/
	{0x1C, 0x80 | (1<<4)},	/*Enter*/
	{0x00, 0}
};

void TranslateScancode(byte scan) {
	int i;
	byte ks = scan & 0x7F;
	keyboard_scan = ks;

	for (i = 0;keyboard_map[i].scan != 0;i++) {
		if (keyboard_map[i].scan == ks) {
			byte action = keyboard_map[i].action;
			if (scan & 0x80) {
				/*release*/
				keyboard_scan = 0;
				action = ~action;
				if ((action & 0x40) == 0)
					keyboard_specials &= action;
				keyboard_arrows &= action & 0x8F;
				keyboard_buttons &= (action >> 4) & 3;
			} else {
				/*mark*/
				if (action & 0x40) {
					keyboard_specials |= action;
					keyboard_specials &= (0x40 | 0x10 | 2 | 1);
					if (keyboard_specials == (0x40 | 0x10 | 2 | 1)) {
						/*TODO: reboot system*/
					}
				} else {
					keyboard_arrows |= action & 0x8F;
					keyboard_buttons |= (action >> 4) & 3;
				}
			}
			return;
		}
	}

	if(scan & 0x80)
		keyboard_scan = 0;
}

void INTERRUPT KeyboardIsr() {
	byte scan, strobe;
	scan = inportb(0x60);
	/*consume scan from kbd. controller*/
	strobe = inportb(0x61);
	outportb(0x61, strobe | 0x80);
	outportb(0x61, strobe);

	TranslateScancode(scan);

	outportb(0x20, 0x20);
}

/*
Detect and initialize joystick
*/
void InitJoystick(void) {
	/*TODO*/
	have_joystick = 0;
}

void InitInput(void) {
	/*disable critical errors handler*/
	setvect(0x24, NullIsr);

	/*is mouse present?*/
	if (getvect(0x33)) {
		union REGS reg;
#ifdef __386__
		reg.w.ax = 0;
		int386(0x33, &reg, &reg);
		if (reg.w.ax == 0xFFFF) {
			/*mouse detected*/

			reg.w.ax = 0xF; /*set cursor speed*/
			reg.w.cx = 16;
			reg.w.dx = 16;
			int386(0x33, &reg, &reg);

			reg.w.ax = 7;   /*set x range*/
			reg.w.cx = 0;
			reg.w.dx = 303;
			int386(0x33, &reg, &reg);

			reg.w.ax = 8;   /*set y range*/
			reg.w.cx = 0;
			reg.w.dx = 183;
			int386(0x33, &reg, &reg);

			reg.w.ax = 4;   /*set coords*/
			cursor_x = reg.w.cx = 10;
			cursor_y = reg.w.dx = 10;
			int386(0x33, &reg, &reg);

			have_mouse = ~0;
		}
#else
		reg.x.ax = 0;
		int86(0x33, &reg, &reg);
		if (reg.x.ax == 0xFFFF) {
			/*mouse detected*/

			reg.x.ax = 0xF; /*set cursor speed*/
			reg.x.cx = 16;
			reg.x.dx = 16;
			int86(0x33, &reg, &reg);

			reg.x.ax = 7;   /*set x range*/
			reg.x.cx = 0;
			reg.x.dx = 303;
			int86(0x33, &reg, &reg);

			reg.x.ax = 8;   /*set y range*/
			reg.x.cx = 0;
			reg.x.dx = 183;
			int86(0x33, &reg, &reg);

			reg.x.ax = 4;   /*set coords*/
			cursor_x = reg.x.cx = 10;
			cursor_y = reg.x.dx = 10;
			int86(0x33, &reg, &reg);

			have_mouse = ~0;
		}
#endif
	}

	if (have_mouse)
		return;

	/*no mouse*/

	InitJoystick();

	old_keyboard_isr = getvect(9);
	setvect(9, KeyboardIsr);
}

void UninitInput(void) {
	if (have_mouse)
		return;

	setvect(9, old_keyboard_isr);
#ifdef DEBUG
	old_keyboard_isr = 0;
#endif
}
