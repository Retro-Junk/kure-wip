import sys
import struct

trans = " !`o au`() +,-.O0123456789:;i=>? ABCDEFGHIJKLMNOPQRSTUVWXYZ||!! "
#        0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef
#        0               1               2               3              3

def ConvString(s):
	res = ""
	for b in s:
		c = b & 0x3F
		f = b & 0xC0
		c = trans[c]
		if f != 0:
			if f == 0x80:
				c += trans[0x25]
			elif f == 0x40:
				c += " "
			else:
				c += trans[0x21]

		res += c
	return res

def DumpFile(filename):
	data = bytearray(open(filename, "rb").read())
	strs = []
	offs = 0
	while offs < len(data):
		l = data[offs]
		#print("%X : %d"%(offs, l))
		if l == 0:
			break
		s = data[offs+1:offs+l]
		strs.append(s)
		offs += l

	print("%d strings loaded"%(len(strs)))

	for i in range(len(strs)):
		s = ConvString(strs[i])
		print("%4d : [%s]"%(i, s))

def LoadFromFile(filename):
	data = bytearray(open(filename, "rb").read())
	strs = []
	offs = 0
	while offs < len(data):
		l = data[offs]
		if l == 0:
			break
		s = data[offs+1:offs+l]
		strs.append(ConvString(s))
		offs += l
	return strs

if __name__ == "__main__":
	DumpFile("VEPCE.BIN")
	#DumpFile("MOTSE.BIN")
	#DumpFile("DESCE.BIN")
	#DumpFile("DIALE.BIN")
