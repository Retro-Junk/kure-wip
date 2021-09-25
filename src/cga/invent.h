#ifndef _INVENT_H_
#define _INVENT_H_

extern byte inv_count;
extern byte inv_bgcolor;

void DrawInventoryBox(uint16 filtermask, uint16 filtervalue);

void OpenInventory(uint16 filtermask, uint16 filtervalue);

#endif
