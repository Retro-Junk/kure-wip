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
				scanline = rast[cgaoddlines + (y / 2) * cgascanbytes:cgaoddlines + (y / 2) * cgascanbytes + cgascanbytes]
			else:
				scanline = rast[(y / 2) * cgascanbytes:(y / 2) * cgascanbytes + cgascanbytes]
		else:
			scanline = rast[y * cgascanbytes:y * cgascanbytes + cgascanbytes]
		ofs = 0
		cpx = 0
		for x in range(w):
			if (x % 4) == 0:
				cpx = scanline[ofs]
				ofs += 1
			pix = ((cpx << ((x % 4) * 2)) >> 6) & 3
			pixels[pxofs] = pix
			pxofs += 1

	return pixels	


def DumpCgaScreen(filename):
	data = bytearray(open(filename, "rb").read())
	SelectCGAPalette(3)

	palette = pngpal
	#alpha = [0,255][opaq]
	#palette[0] = (palette[0][0], palette[0][1], palette[0][2], alpha)

	raster = UnpackCGARaster(data, w = 320, h = 200, rw = 320, rh = 200, interlaced = True, padded = True)
	#open("tata.raw", "wb").write(raster)

	ofs = 0
	lines = []

	for hh in range(200):
		line = []
		for ww in range(320):
			line.append(raster[ofs] & 0x3)
			ofs += 1
		line = line[:320]
		lines.append(line)
	WritePng(320, 200, data, palette, filename + ".png", lines)

if len(sys.argv) > 1:
	filename = sys.argv[1]
	DumpCgaScreen(filename)
