import sys
import struct

def DumpRoom(data):
	ofs = 1
	while ofs < len(data):
		spridx, x, y = struct.unpack_from("BBB", data, ofs)

		xx = (x & 0x7F) * 4
		yy = (y & 0x7F) * 2
		xflip = x & 0x80 != 0
		yflip = y & 0x80 != 0

		fmt = " %3d @ %d:%d"%(spridx, xx, yy)
		if xflip:
			fmt += ", XFlip"
		if yflip:
			fmt += ", YFlip"
		print(fmt)

		ofs += 3

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
		print("Room %d"%(i + 1))
		DumpRoom(blks[i])
		print("")

ProcessFile("ARPLA.BIN")
