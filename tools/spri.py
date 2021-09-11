import sys
import os
import struct

import png

pngpal = []

cga_palettes = [
	[(0x00,0x00,0x00), (0x00,0xAA,0x00), (0xAA,0x00,0x00), (0xAA,0x55,0x00)],	# 0
	[(0x00,0x00,0x00), (0x55,0xFF,0x55), (0xFF,0x55,0x55), (0xFF,0xFF,0x55)],	# 0i
	[(0x00,0x00,0x00), (0x00,0xAA,0xAA), (0xAA,0x00,0xAA), (0xAA,0xAA,0xAA)],	# 1
	[(0x00,0x00,0x00), (0x55,0xFF,0xFF), (0xFF,0x55,0xFF), (0xFF,0xFF,0xFF)],	# 1i
	[(0x00,0x00,0x00), (0x00,0xAA,0xAA), (0xAA,0x00,0x00), (0xAA,0xAA,0xAA)],	# 2
	[(0x00,0x00,0x00), (0x55,0xFF,0xFF), (0xFF,0x55,0x55), (0xFF,0xFF,0xFF)] 	# 2i
	]

def SelectCGAPalette(index = 0, black = (0,0,0)):
	global pngpal
	pngpal = []
	for i in range(256):
		if i < 4:
			if i == 0:
				r = black[0]
				g = black[1]
				b = black[2]
			else:
				r = cga_palettes[index][i][0]
				g = cga_palettes[index][i][1]
				b = cga_palettes[index][i][2]
			a = 255
		else:
			r = 0
			g = 0
			b = 0
			a = 0
		pngpal.append((r, g, b, a))

def WritePng(w, h, pixels, palette, filename, lines=None):
	with open(filename, "wb+") as f:
		wr = png.Writer(w, h, palette=palette, bitdepth=8)
		if True:
			if not lines:
				lines = []
				for hh in range(h):
					lines.append(pixels[hh*w:hh*w+w])
			wr.write_packed(f, lines)
		else:
			wr.write(f, pixels)

def UnpackCGARaster(rast, w = 320, h = 200, rw = 320, rh = 200, interlaced = True, padded = True):
	cgascanbytes = rw // 4
	cgaoddlines = cgascanbytes * (rh // 2)
	if padded:
		cgaoddlines = 0x2000

	pixels = bytearray(w * h)
	pxofs = 0
	for y in range(rh):
		#print("line %d"%y)
		if interlaced:
			if y & 1:
				lstart = (y // 2) * cgascanbytes + cgaoddlines
			else:
				lstart = (y // 2) * cgascanbytes
		else:
			lstart = y * cgascanbytes

		#print("y = %d, lstart = %d , sb = %d , rl = %d"%(y, lstart, cgascanbytes, len(rast)))
		
		scanline = rast[lstart:lstart + cgascanbytes]

		ofs = 0
		cpx = 0
		pxofs = y * w
		for x in range(rw):
			if (x % 4) == 0:
				cpx = scanline[ofs]
				ofs += 1
			pix = ((cpx << ((x % 4) * 2)) >> 6) & 3
			pixels[pxofs] = pix
			pxofs += 1

	return pixels	

def ProcessSprite(spr):

	w, h = struct.unpack_from("BB", spr, 0)
	print("size = %d x %d (%d x %d), rofs = 0x%X"%(w, h, w * 4, h, 2 + w * h))

	part1 = spr[2:2 + w * h]
	part2 = spr[2 + w * h:]

	# part 1 - pixels
	# part 2 - mask

	raster = bytearray(w * h * 2)

	"""
		Draw:
			*screen &= pixel.mask
			*screen |= pixel.pixl
	"""

	if part2[0] == 0:
		# solid
		for i in range(w * h):
			raster[i * 2 + 0] = 0			# mask
			raster[i * 2 + 1] = part1[i]	# pixels
	else:
		mi = 1
		mo = 0

		def MaskBit():
			nonlocal mi, mo, part2
			mi >>= 1
			if mi == 0:
				mi = 0x80
				mo += 1
			return (part2[mo] & mi) != 0

		for i in range(w * h):
			pixels = part1[i]
			mask = 0

			if pixels & 0xC0 == 0:	# color 0
				if MaskBit():		# transparent?
					mask |= 0xC0	# yes

			if pixels & 0x30 == 0:	# color 1
				if MaskBit():		# transparent?
					mask |= 0x30	# yes

			if pixels & 0x0C == 0:	# color 2
				if MaskBit():		# transparent?
					mask |= 0x0C	# yes

			if pixels & 0x03 == 0:	# color 3
				if MaskBit():		# transparent?
					mask |= 0x03	# yes

			raster[i * 2 + 0] = mask
			raster[i * 2 + 1] = pixels

	return w, h, raster

def ProcessFile(filename):
	data = bytearray(open(filename, "rb").read())
	entries = []
	offs = 4
	while offs < len(data):
		l = struct.unpack_from("<H", data, offs)[0]
		#print("%X : %d"%(offs, l))
		if l == 0:
			break
		s = data[offs+2:offs+l]
		entries.append(s)
		offs += l

	print("%d entries loaded"%(len(entries)))

	SelectCGAPalette(3)
	palette = pngpal

	for index in range(len(entries)):
		w, h, spraster = ProcessSprite(entries[index])
		rast = bytearray()
		for i in range(w * h):
			rast.append(spraster[i * 2 + 1])

		w *= 4

		raster = UnpackCGARaster(rast, w, h, rw = w, rh = h, interlaced = False, padded = False)
		#open("tata.raw", "wb").write(raster)

		ofs = 0
		lines = []

		for hh in range(h):
			line = []
			for ww in range(w):
				line.append(raster[ofs] & 0x3)
				ofs += 1
			line = line[:w]
			lines.append(line)

		WritePng(w, h, data, palette, filename + ".%d.png"%index, lines)
		
ProcessFile("PERS2.BIN")

