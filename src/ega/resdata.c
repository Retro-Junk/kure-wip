#include <io.h>
#include <fcntl.h>
#include "common.h"
#include "resdata.h"
#include "decompr.h"

extern void AskDisk2(void);
extern int LoadSplash(const char *filename);

/*
Get bank entry
TODO: port SeekToString to this routine
*/
byte *SeekToEntry(byte *bank, unsigned int num, byte **end) {
	byte len;
	byte *p = bank;

	while (num--) {
		len = *p;
		p += len;
	}
	len = *p;
	*end = p + len;
	return p + 1;
}

byte *SeekToEntryW(byte *bank, unsigned int num, byte **end) {
	unsigned int len;
	byte *p = bank;

	while (num--) {
		len = p[0] | (p[1] << 8);
		p += len;
	}
	len = p[0] | (p[1] << 8);
	*end = p + len;
	return p + 2;
}

unsigned int LoadFile(const char *filename, byte *buffer) {
	int f;
	int rlen;
	f = open(filename, O_RDONLY | O_BINARY);
	if (f == -1)
		return 0;
	rlen = read(f, buffer, 0xFFF0);
	close(f);
	if (rlen == -1)
		return 0;
	return (unsigned int)rlen;
}

unsigned int SaveFile(char *filename, byte *buffer, unsigned int size) {
	int f;
	int wlen;
	f = open(filename, O_RDONLY | O_BINARY);
	if (f == -1)
		return 0;
	wlen = write(f, buffer, size);
	close(f);
	if (wlen == -1)
		return 0;
	return (unsigned int)wlen;
}

int LoadFilesList(ResEntry_t *entries) {
	int i;
	for (i = 0; entries[i].name[0] != '$'; i++) {
		if (!LoadFile(entries[i].name, entries[i].buffer))
			return 0;
	}
	return 1;
}


byte arpla_data[RES_ARPLA_MAX];
byte aleat_data[RES_ALEAT_MAX];
byte icone_data[RES_ICONE_MAX];
byte souco_data[RES_SOUCO_MAX];
byte souri_data[RES_SOURI_MAX];
byte mursm_data[RES_MURSM_MAX];
byte gauss_data[RES_GAUSS_MAX];
byte lutin_data[RES_LUTIN_MAX];
byte anima_data[RES_ANIMA_MAX];
byte anico_data[RES_ANICO_MAX];
byte zones_data[RES_ZONES_MAX];

ResEntry_t res_static[] = {
	{"ARPLA.BIN", arpla_data},
	{"ALEAT.BIN", aleat_data},
	{"ICONE.BIN", icone_data},
	{"SOUCO.BIN", souco_data},
	{"CARPC.BIN", carpc_data},
	{"SOURI.EGA", souri_data},
	{"TEMPL.EGA", templ_data},
	{"MURSM.BIN", mursm_data},
	{"GAUSS.EGA", gauss_data},
	{"LUTIN.BIN", lutin_data},
	{"ANIMA.BIN", anima_data},
	{"ANICO.BIN", anico_data},
	{"ZONES.BIN", zones_data},
	{"$"}
};

/*
Load resident data files. Original game has all these data files embedded in the executable.
NB! Static data includes the font file, don't use any text print routines before it's loaded.
*/
int LoadStaticData() {
	return LoadFilesList(res_static);
}

ResEntry_t res_texts[] = {
	{"DIALE.BIN", diali_data},
	{"DESCE.BIN", desci_data},
	{"VEPCE.BIN", vepci_data},
	{"MOTSE.BIN", motsi_data},
	{"$"}
};

/*
Load strings data
*/
int LoadTextData() {
	return LoadFilesList(res_texts);
}

int LoadFond(void) {
	return LoadSplash("FOND.EGA");
}

ResEntry_t res_sprites[] = {
	{"PERSO.EGA", perso_data},
	{"SPRIT.EGA", sprit_data},
	{"$"}
};

int LoadSpritesData(void) {
	return LoadFilesList(res_sprites);
}

ResEntry_t res_puzzle[] = {
	{"PUZZL.EGA", puzzl_data},
#ifdef FIXME
	{"PUZZ1.EGA", puzz1_data},
#endif
	{"$"}
};

int LoadPuzzlData(void) {
	return LoadFilesList(res_puzzle);
}
