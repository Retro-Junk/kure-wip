import sys
import struct

class MZImage:
	def __init__(self, buffer):
		self.ParseImage(buffer)

	def ParseImage(self, buffer):
		buffer = bytearray(buffer)
		sig, lastpg, pages, relocs, hdrparas, minmem, maxmem, ss, sp, crc, ip, cs, relofs, ovlnum = struct.unpack_from("<HHHHHHHHHHHHHH", buffer, 0)
		self.image = buffer[hdrparas*16:]

class LZ78Decoder:

	def __init__(self):
		self.stack = []

	def decode_string(self, prev_n, code):
		while self.work1[code] != 0:
			n = self.work1[code]
			while n >= prev_n:
				n = self.work2[n]
				if n == 0:
					return code

			code = self.dict1[n]
			suffix = self.dict2[n]
			self.stack.append((n, suffix))
			prev_n = n

		return code

	def decompress_block(self, data):
		more = 0
		result = bytearray()
		data = bytearray(data)

		dict_size, more, compsize = struct.unpack_from("<BBH", data, 0)
		blocksize = 4 + compsize
		if dict_size == 0:
			# plain data
			result = data[4:4+compsize]
		else:
			# compressed data
			blocksize += dict_size * 3
			dp = 4
			self.dict0 = [0] + list(data[dp:dp+dict_size])
			dp += dict_size
			self.dict1 = [0] + list(data[dp:dp+dict_size])
			dp += dict_size
			self.dict2 = [0] + list(data[dp:dp+dict_size])
			dp += dict_size

			self.work1 = [0] * 256
			self.work2 = [0] * 256
			for i in range(1, dict_size + 1):
				code = self.dict0[i]
				self.work2[i] = self.work1[code]
				self.work1[code] = i

			while compsize > 0:
				code = data[dp]
				dp += 1
				compsize -= 1

				if self.work1[code] == 0:
					# literal
					result.append(code)
				else:
					# string
					self.stack = []
					n = self.work1[code]

					code = self.dict1[n]
					suffix = self.dict2[n]
					self.stack.append((n, suffix))

					while True:
						suffix = self.decode_string(n, code)
						result.append(suffix)
						if len(self.stack) == 0:
							break
						(n, code) = self.stack.pop()

		return result, more, blocksize

def decompress(data):
	result = b""
	offset = 0
	more = True

	lz = LZ78Decoder()
	while more:
		res, more, compsize = lz.decompress_block(data[offset:])
		result += res
		offset += compsize

	return result

def ExtractResources(mod_data, index, first_res):
	print("Extracting internal resources")
	mz = MZImage(mod_data)
	resofs = mz.image.find(first_res)
	if resofs == -1:
		print("No internal resources table found")
	else:
		print("Found resources table at 0x%X"%resofs)
		restable = []
		while mz.image[resofs] != ord('$'):
			resname, reso, ress = struct.unpack_from("<10sHH", mz.image, resofs)
			restable.append((resname.split(b"\0")[0].decode("ascii"), ress*16 + reso))
			resofs += 5 + 1 + 3 + 1 + 4
		restable = sorted(restable, key=lambda r:r[1])
		#print(restable)
		for i in range(len(restable)):
			rname = restable[i][0]
			rbeg = restable[i][1]
			rend = restable[i+1][1] if i != len(restable) - 1 else len(mz.image)
			print("%10s : %X %d"%(rname, rbeg, rend - rbeg))
			rdata = mz.image[rbeg:rend]
			open("%d_%s"%(index, rname), "wb").write(rdata)

def dump_pxi_file(filename, extractres=False):
	filedata = open(filename, "rb").read()

	num_mods = struct.unpack_from(">H", filedata, 0)[0]
	mod_base = 2 + num_mods * 4

	mod_offs = [mod_base + struct.unpack_from(">I", filedata, 2 + i * 4)[0] for i in range(num_mods)]

	for i in range(len(mod_offs)):
		mod_ofs = mod_offs[i]
		mod_psize, mod_usize = struct.unpack_from(">II", filedata, mod_ofs)
		mod_data = filedata[mod_ofs + 8:mod_ofs + 8 + mod_psize]
		print("Module %d : at 0x%6X, psize = %6d, usize = %6d"%(i, mod_ofs, mod_psize, mod_usize))

		mod_data = decompress(mod_data)
		print("decoded to %d bytes"%(len(mod_data)))
		open(filename + ".out.%d.bin"%i, "wb").write(mod_data)	

		if extractres:
			if not mod_data.startswith(b"MZ"):
				print("Module decompressed, but appears to be invalid")
			else:
				ExtractResources(mod_data, i, b"ARPLA.BIN")

def dump_bin_file(filename):
	filedata = open(filename, "rb").read()
	psize, usize = struct.unpack_from(">II", filedata, 0)
	data = filedata[8:8 + psize]
	print("psize = %6d, usize = %6d"%(psize, usize))

	data = decompress(data)
	print("decoded to %d bytes"%(len(data)))
	open(filename + ".out.bin", "wb").write(data)	

if __name__ == "__main__":

	if len(sys.argv) > 2:
		dump_pxi_file(sys.argv[1], len(sys.argv) > 3)
	elif len(sys.argv) > 1:
		dump_bin_file(sys.argv[1])
	else:
		print("usage: %s file         -- to decompress binary file"%sys.argv[0])
		print("usage: %s file pxi     -- to decompress pxi bundle file"%sys.argv[0])
		print("usage: %s file pxi res -- to decompress pxi bundle file and extract embedded resources"%sys.argv[0])
		print("  some resources may still need to be trimmed manually")
