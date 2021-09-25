#ifndef _MENU_H_
#define _MENU_H_

extern byte act_menu_x;
extern byte act_menu_y;

void ActionsMenu(byte **pinfo);
void MenuLoop(byte spotmask, byte spotvalue);
void ProcessMenu(void);

void CheckMenuCommandHover(void);
void CheckPsiCommandHover(void);

#endif
