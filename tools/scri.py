import sys
import struct
import stri

diali = stri.LoadFromFile("DIALE.BIN")
vepci = stri.LoadFromFile("VEPCE.BIN")
motsi = stri.LoadFromFile("MOTSE.BIN")
desci = stri.LoadFromFile("DESCE.BIN")

maxb = 0
maxw = 0

def ScriptVar(block, offs):
	global maxb
	global maxw

	line = ""
	detail = {}
	vt = block[offs]
	offs += 1

	detail["Variable"] = (vt & 0x80) != 0
	detail["Indexed"] = (vt & 0x40) != 0
	detail["Word"] = (vt & 0x20) != 0
	detail["Kind"] = vt & 0x1F

	if vt & 0x80:
		if vt & 0x40:
			index = block[offs]
			offs += 1
			detail["Index"] = index
		vofs = block[offs]
		offs += 1
		detail["Offset"] = vofs

		if vt & 0x40:
			line = "$%02X[%02X].%X"%(detail["Kind"], detail["Index"], detail["Offset"])
		else:
			line = "$%02X.%X"%(detail["Kind"], detail["Offset"])
			if detail["Kind"] == 1:
				maxw = max(maxw, detail["Offset"])
			elif detail["Kind"] == 2:
				maxb = max(maxb, detail["Offset"])
	else:
		value = block[offs]
		offs += 1
		if vt & 0x20:
			value = (value << 8) | block[offs]
			offs += 1
		detail["Value"] = value
		line = "#0x%X"%value

	if vt & 0x20:
		line += "w"
		
	return offs, line, detail

def DecodeMathOp(op):
	res = []
	if op & 0x40:
		if op & 0x20:
			res.append("==")
		if op & 0x10:
			res.append("<")
		if op & 0x8:
			res.append(">")
		if op & 0x4:
			res.append("!=")
		if op & 0x2:
			res.append("<~")
		if op & 0x1:
			res.append(">~")
		if len(res) == 0:
			res.append("NULL")
	else:
		if op & 0x20:
			res.append("+")
		if op & 0x10:
			res.append("-")
		if op & 0x8:
			res.append("&")
		if op & 0x4:
			res.append("|")
		if op & 0x2:
			res.append("^")
		if len(res) == 0:
			res.append("SAME")

	return " ".join(res)

def MathExpr(block, offs):
	expr = ""
	offs, v1, _ = ScriptVar(block, offs)
	expr = v1
	while True:
		op = block[offs]
		offs += 1
		if op & 0x80:
			break
		offs, v2, _ = ScriptVar(block, offs)
		expr += " " + DecodeMathOp(op) + " " + v2
	return offs, expr


def SH_CondExpr(block, offs, line, disp):
	offs, expr = MathExpr(block, offs)
	targ = (block[offs + 1] << 8) | block[offs];
	offs += 2
	return offs, line + disp["m"] + " " + expr + " :=> 0x%X"%targ

def SH_MathExpr(block, offs, line, disp):
	offs, v1, _ = ScriptVar(block, offs)
	offs, expr = MathExpr(block, offs)
	return offs, line + disp["m"] + " " + v1 + " := " + expr

def SH_Byte(block, offs, line, disp):
	value = block[offs];
	offs += 1
	return offs, line + disp["m"] + " 0x%X"%value

def SH_Word(block, offs, line, disp):
	value = (block[offs + 1] << 8) | block[offs];
	offs += 2
	return offs, line + disp["m"] + " 0x%X"%value

def SH_Call(block, offs, line, disp):
	targ = (block[offs + 1] << 8) | block[offs];
	offs += 2
	return offs, line + disp["m"] + " :=> 0x%X"%targ

def SH_SelectSpot(block, offs, line, disp):
	value1 = block[offs];
	value2 = block[offs + 1];
	offs += 2
	return offs, line + disp["m"] + " m:0x%X, v:0x%X"%(value1, value2)

def SH_IconDraw(block, offs, line, disp):
	index = block[offs];
	offs += 1
	if index == 255:
		return offs, line + disp["m"] + " Current"
	coords = (block[offs + 1] << 8) | block[offs];
	offs += 2
	return offs, line + disp["m"] + " icon:0x%X, coords:0x%X (%d:%d)"%(index, coords, (coords % 256) * 4, coords // 256)

def SH_IconZoom(block, offs, line, disp):
	index = block[offs];
	offs += 1
	if index == 255:
		return offs, line + disp["m"] + " Current"
	coords = (block[offs + 1] << 8) | block[offs];
	offs += 2

	zoomw = block[offs]
	zoomh = block[offs + 1]
	offs += 2

	return offs, line + disp["m"] + " icon:0x%X, coords:0x%X (%d:%d) zoom:%d,%d"%(index, coords, (coords % 256) * 4, coords // 256, zoomw, zoomh)

def SH_DrawText(block, offs, line, disp):
	index = block[offs];
	offs += 1
	if index < 4:
		index = (index << 8) | block[offs]
		offs += 1
	coords = (block[offs + 1] << 8) | block[offs]
	offs += 2
	width = block[offs];
	offs += 1
	return offs, line + disp["m"] + " text:0x%X, coords:0x%X, flags_width:0x%X"%(index, coords, width)  + ' -> "%s"'%disp["t"][index - 4]

def SH_TextIndex(block, offs, line, disp):
	index = block[offs];
	offs += 1
	if index < 4:
		index = (index << 8) | block[offs]
		offs += 1
	return offs, line + disp["m"] + " text:0x%X"%(index) + ' -> "%s"'%disp["t"][index - 4]

def SH_FindInvItem(block, offs, line, disp):
	first = block[offs];
	offs += 1
	count = block[offs];
	offs += 1
	value = block[offs];
	offs += 1
	return offs, line + disp["m"] + " first:0x%X, count:0x%X, value:0x%X"%(first, count, value)

def SH_Triplet(block, offs, line, disp):
	value1 = block[offs];
	offs += 1
	value2 = block[offs];
	offs += 1
	value3 = block[offs];
	offs += 1
	return offs, line + disp["m"] + " v1:0x%X, v2:0x%X, v3:0x%X"%(value1, value2, value3)

def SH_RunAnim(block, offs, line, disp):
	value1 = block[offs];
	offs += 1
	value2 = block[offs];
	offs += 1
	value3 = block[offs];
	offs += 1
	return offs, line + disp["m"] + " v1:0x%X, v2:0x%X, v3:0x%X"%(value1, value2, value3)

def SH_ActionsMenu(block, offs, line, disp):
	buttons = block[offs];
	offs += 1

	choices = []
	for i in range(8):
		if buttons & (1 << i):
			hint = block[offs]
			offs += 1
			command = (block[offs] << 8) | block[offs + 1];
			offs += 2
			choices.append((hint, command))

	choices = ", ".join(['$%04X:%d "%s"'%(c[1], c[0], disp["t"][c[0] - 4]) for c in choices])

	return offs, line + disp["m"] + " buttons:0x%X, choices:[%s]"%(buttons, choices)

def SH_RoomSprite(block, offs, line, disp):
	sprite = block[offs];
	offs += 1
	x = block[offs];
	offs += 1
	y = block[offs];
	offs += 1

	flags = ""
	if x & 0x80:
		flags += ", XFlip"
	if y & 0x80:
		flags += ", YFlip"

	xx = (x & 0x7F) * 4
	yy = (y & 0x7F) * 2

	return offs, line + disp["m"] + " sprite:%d, x:%d, y:%d"%(sprite, xx, yy) + flags


script_handlers = {
0x01:{"m":"KindAspirantTrade"},
0x02:{"m":"RudeAspirantTrade"},
0x03:{"m":"DrawDesciForItem", "h":SH_Byte},     # box with item and its name
0x04:{"m":"StealZapstik"},
0x05:{"m":"DrawPortraitLiftRight", "h":SH_IconDraw},
0x06:{"m":"DrawPortraitLiftLeft", "h":SH_IconDraw},
0x07:{"m":"DrawPortraitLiftDown", "h":SH_IconDraw},
0x08:{"m":"DrawPortraitLiftUp", "h":SH_IconDraw},
0x09:{"m":"DrawPortrait", "h":SH_IconDraw},
0x0B:{"m":"DrawPortraitTwist", "h":SH_IconDraw},
0x0C:{"m":"DrawPortraitArc", "h":SH_IconDraw},
0x0D:{"m":"DrawPortraitDotEffect", "h":SH_IconDraw},
0x0E:{"m":"DrawPortraitZoomIn", "h":SH_IconDraw},
0x10:{"m":"DrawPortraitZoomed", "h":SH_IconZoom},
0x11:{"m":"DrawRoomObject", "h":SH_RoomSprite},
0x12:{"m":"Chain", "h":SH_Word},				# jump to another subroutine
0x13:{"m":"RedrawRoomStatics", "h":SH_Byte},
0x14:{"m":"DrawDesciText", "h":SH_TextIndex, "t":desci},
0x15:{"m":"SelectSpot", "h":SH_SelectSpot},
0x16:{"m":"DrawDeathAnim"},
0x17:{"m":"DrawPersonThoughtBubbleDialog", "h":SH_TextIndex, "t":diali},
0x18:{"m":"AnimPortrait", "h":SH_Triplet},
0x19:{"m":"HidePortraitLiftLeft", "h":SH_Byte},
0x1A:{"m":"HidePortraitLiftRight", "h":SH_Byte},
0x1B:{"m":"HidePortraitLiftUp", "h":SH_Byte},
0x1C:{"m":"HidePortraitLiftDown", "h":SH_Byte},
0x1E:{"m":"HidePortraitTwist", "h":SH_Byte},
0x1F:{"m":"HidePortraitArc", "h":SH_Byte},
0x20:{"m":"HidePortraitDots", "h":SH_Byte},
0x22:{"m":"HidePortraitShatter", "h":SH_Byte},
0x23:{"m":"HidePortrait", "h":SH_Byte},
0x24:{"m":"PopAllPortraits"},
0x25:{"m":"ChangeZoneOnly", "h":SH_Byte},
0x26:{"m":"GameOver"},
0x27:{"m":"DrawGaussBubble", "h":SH_TextIndex, "t":diali},
0x28:{"m":"MenuLoop", "h":SH_Triplet},
0x29:{"m":"DrawDialiBubble", "h":SH_DrawText, "t":diali},
0x2A:{"m":"PopBubble", "h":SH_Byte},
0x2B:{"m":"PopAllBubbles"},
0x2C:{"m":"Wait4"},
0x2D:{"m":"Wait", "h":SH_Byte},
0x2E:{"m":"PromptWait"},
0x2F:{"m":"TakePersonsItem"},
0x30:{"m":"Fight"},
0x31:{"m":"Fight2"},
0x32:{"m":"FightWin"},
0x33:{"m":"Jump", "h":SH_Call},
0x34:{"m":"Call", "h":SH_Call},
0x35:{"m":"Ret"},
0x36:{"m":"ChangeZone", "h":SH_Byte},
0x37:{"m":"DrawDesciTextBox", "h":SH_DrawText, "t":desci},	# draw box with text
0x38:{"m":"RunAnim", "h":SH_RunAnim},
0x39:{"m":"AnimRoomDoorOpen", "h":SH_Byte},
0x3A:{"m":"AnimRoomDoorClose", "h":SH_Byte},
0x3B:{"m":"MathExpr", "h":SH_MathExpr},
0x3C:{"m":"CondExpr", "h":SH_CondExpr},
0x3D:{"m":"ActionsMenu", "h":SH_ActionsMenu, "t":vepci},
0x3E:{"m":"TheWallAdvance"},
0x40:{"m":"PopAllTextBoxes"},
0x41:{"m":"LiftHand"},
0x42:{"m":"LoadZone", "h":SH_Byte},
0x43:{"m":"RefreshZone"},
0x44:{"m":"BackBufferToScreen"},
0x45:{"m":"DeProfundisRoomEntry"},
0x46:{"m":"DeProfundisLowerHook"},
0x47:{"m":"DeProfundisRiseMonster"},
0x48:{"m":"DeProfundisLowerMonster"},
0x49:{"m":"DeProfundisRiseHook"},
0x4B:{"m":"ProtoDropZapstik"},
0x4C:{"m":"DrawZoneObjs"},
0x4D:{"m":"PriorityCommand", "h":SH_Word},	# discard current callstack and run the command (Fxxx)
0x4E:{"m":"CurrentItemFlyToRoom"},
0x4F:{"m":"CurrentItemFlyToInventory"},
0x50:{"m":"ItemFlyToInventory", "h":SH_Byte},
0x51:{"m":"ItemTrade"},
0x52:{"m":"RefreshSpritesData"},
0x53:{"m":"FindInvItem", "h":SH_FindInvItem},
0x54:{"m":"DotFadeRoom"},
0x55:{"m":"DrawRoomItemsIndicator"},
0x56:{"m":"MorphRoom98"},
0x57:{"m":"ShowCharacterSprite", "h":SH_Triplet},
0x58:{"m":"DrawCharacterSprite", "h":SH_Triplet},
0x59:{"m":"BlitSpritesToBackBuffer"},
0x5A:{"m":"SelectPalette"},
0x5B:{"m":"TheEnd"},
0x5C:{"m":"ClearInventory"},
0x5D:{"m":"DropWeapons"},
0x5E:{"m":"SelectTempPalette", "h":SH_Byte},
0x5F:{"m":"DrawRoomObjectBack", "h":SH_RoomSprite},
0x60:{"m":"ReviveCadaver"},
0x61:{"m":"DrawPersonBubbleDialog", "h":SH_TextIndex, "t":diali},
0x62:{"m":"PsiReaction", "h":SH_Byte},
0x63:{"m":"LiftSpot6"},
0x64:{"m":"DrawBoxAroundSpot"},
0x65:{"m":"DeProfundisMovePlatform", "h":SH_Byte},
0x66:{"m":"DeProfundisRideToExit"},
0x67:{"m":"Unused67", "h":SH_Byte},
0x68:{"m":"Unused68", "h":SH_Byte},
0x69:{"m":"PlaySound", "h":SH_Word},
0x6A:{"m":"Unused6A"},
0xFF:{"m":"End"}
}

unimplemented = set()

def DisasmRoutine(block, baseofs = 0):
	global unimplemented

	offs = 1
	while offs < len(block):
		opcode = block[offs]
		line = "%04X: %02X "%(offs + baseofs, opcode)
		if opcode == 0:
			line += "END"
			print("%s"%line)
			break

		disp = script_handlers.get(opcode)
		if not disp:
			unimplemented.add(opcode)
			line += "<not implemented>"
			print("%s"%line)
			break

		handler = disp.get("h")
		if handler:
			offs, line = handler(block, offs + 1, line, disp)
		else:
			line += disp["m"]
			offs += 1

		print("%s"%line)
		
	if offs != len(block):
		print("premature script end (at 0x%X, len 0x%X)"%(offs, len(block)))
	else:
		print("*end*")

def ProcessFile(filename):
	global unimplemented

	data = bytearray(open(filename, "rb").read())
	blks = []
	bofs = []
	offs = 0
	while offs < len(data):
		l = data[offs]
		#print("%X : %d"%(offs, l))
		if l == 0:
			break
		s = data[offs:offs+l]
		blks.append(s)
		bofs.append(offs)
		offs += l

	print("%d code blocks loaded"%(len(blks)))

	#DisasmRoutine(blks[0])

	for i in range(len(blks)):
		print("%04X: Routine %d (0x%X)"%(bofs[i], i + 1, i + 1))
		DisasmRoutine(blks[i], bofs[i])
		print("")

	print("\n\nMaxB = 0x%X\nMaxW = 0x%X"%(maxb, maxw))
	print("Unimplemented opcodes: %s"%(", ".join(["%02X"%x for x in sorted(list(unimplemented))])))

ProcessFile("TEMPL.BIN")
