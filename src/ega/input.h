#ifndef _INPUT_H_
#define _INPUT_H_

extern byte buttons;
extern byte right_button;

extern byte have_mouse;

extern volatile byte key_direction;
extern volatile byte key_code;
extern byte key_held;

byte ReadKeyboardChar(void);
void ClearKeyboard(void);
byte GetKeyScan(void);

byte PollMouse(unsigned int *curs_x, uint8 *curs_y);
byte PollKeyboard(void);
void SetInputButtons(byte keys);

void PollInput(void);
void PollDiscrete(void);
void ProcessInput(void);
void PollInputButtonsOnly(void);
void ResetInput(void);

void InitInput(void);
void UninitInput(void);

#endif
