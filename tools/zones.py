import sys
import struct
import stri

diali = stri.LoadFromFile("DIALE.BIN")
vepci = stri.LoadFromFile("VEPCE.BIN")
motsi = stri.LoadFromFile("MOTSE.BIN")
desci = stri.LoadFromFile("DESCE.BIN")

def GetString(bank, index):
	if index < 4:
		return ""
	return bank[index - 4]

def LoadSpot(data, ofs):
	spot = {}
	sx, ex, sy, ey, flags, name, command = struct.unpack_from(">BBBBBBH", data, ofs)
	ofs += 4 + 1 + 1 + 2

	sx = int(sx) * 4
	ex = int(ex) * 4
	sy = int(sy)
	ey = int(ey)
	flags = int(flags)
	name = int(name)
	command = int(command)

	spot = {"sx":sx,"ex":ex,"sy":sy,"ey":ey,"flags":flags,"name":name,"command":command}

	return spot, ofs

def DumpZone(data):
	ofs = 1
	while ofs < len(data):
		area, room, name, palette, nspots = struct.unpack_from("BBBBB", data, ofs)
		ofs += 5

		print("Area: %d"%area)
		print("Room: %d"%room)
		print('Name: %d "%s"'%(name, GetString(motsi, name)))
		print("Palette: %d"%palette)
		print("Spots: %d"%nspots)

		reactions = []
		for i in range(nspots):
			cmds = [0] * 5

			mask = struct.unpack_from(">H", data, ofs)[0]
			ofs += 2

			if mask & 0x10:
				cmds[0] = struct.unpack_from(">H", data, ofs)[0]
				ofs += 2

			if mask & 8:
				cmds[1] = struct.unpack_from(">H", data, ofs)[0]
				ofs += 2

			if mask & 4:
				cmds[2] = struct.unpack_from(">H", data, ofs)[0]
				ofs += 2

			if mask & 2:
				cmds[3] = struct.unpack_from(">H", data, ofs)[0]
				ofs += 2

			if mask & 1:
				cmds[4] = struct.unpack_from(">H", data, ofs)[0]
				ofs += 2

			reactions.append(cmds)

		# spots follows

		if nspots > 0:
			print(" #:  sx: sy -  ex: ey Flgs Comand Fngr,Bwrp,KMnd,Shft,Evil Name")

		spotindex = 0
		while ofs < len(data):
			spot, ofs = LoadSpot(data, ofs)
			reaction = ",".join(["%04X"%x if x != 0 else "----" for x in reactions[spotindex]])
			name = '%3d "%s"'%(spot["name"], GetString(motsi, spot["name"]))
			flags = ""	# todo
			print("%2d: %3d:%3d - %3d:%3d 0x%02X 0x%04X %s %s %s"%(spotindex, spot["sx"], spot["sy"], spot["ex"], spot["ey"], spot["flags"], spot["command"], reaction, name, flags))
			spotindex += 1

		if spotindex != nspots:
			print("*** check me!")

def ProcessFile(filename):
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

	print("%d blocks loaded"%(len(blks)))

	for i in range(len(blks)):
		print("Zone %d"%(i + 1))
		DumpZone(blks[i])
		print("")

ProcessFile("ZONES.BIN")
