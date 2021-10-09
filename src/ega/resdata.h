#ifndef _RESDATA_H_
#define _RESDATA_H_

typedef struct ResEntry_t {
	char    name[5 + 1 + 3 + 1];
	void    *buffer;
} ResEntry_t;

/* Max resource file size among all languages */
#if defined(VERSION_US_EGA)
#define RES_ALEAT_MAX 256
#define RES_ANICO_MAX 667
#define RES_ANIMA_MAX 2046
#define RES_ARPLA_MAX 8003
#define RES_CARPC_MAX 384
#define RES_GAUSS_MAX 2880
#define RES_ICONE_MAX 2756
#define RES_LUTIN_MAX 2800
#define RES_MURSM_MAX 76
#define RES_SOUCO_MAX 424
#define RES_SOURI_MAX 576
#define RES_TEMPL_MAX 27337
#define RES_ZONES_MAX 9014

#define RES_PUZZL_MAX 37902u
#ifdef FIXME
#define RES_PUZZ1_MAX 42784u
#else
#define RES_PUZZ1_MAX 2u
#endif
#define RES_SPRIT_MAX 34274u
#define RES_PERSO_MAX 38274u

#define RES_DESCI_MAX 10515
#define RES_DIALI_MAX 9636
#define RES_MOTSI_MAX 1082
#define RES_VEPCI_MAX 1345
#elif defined(VERSION_US_CGA) || defined(VERSION_US_HERC)
#define RES_ALEAT_MAX 256
#define RES_ANICO_MAX 667
#define RES_ANIMA_MAX 2046
#define RES_ARPLA_MAX 7910
#define RES_CARPC_MAX 384
#define RES_GAUSS_MAX 1449
#define RES_ICONE_MAX 2756
#define RES_LUTIN_MAX 2800
#define RES_MURSM_MAX 76
#define RES_SOUCO_MAX 424
#define RES_SOURI_MAX 1152
#define RES_TEMPL_MAX 27337
#define RES_ZONES_MAX 9014
#define RES_PUZZL_MAX 45671
#define RES_SPRIT_MAX 23811
#define RES_PERS1_MAX 14294
#define RES_PERS2_MAX 10587
#define RES_DESCI_MAX 10515
#define RES_DIALI_MAX 9636
#define RES_MOTSI_MAX 1082
#define RES_VEPCI_MAX 1345
#else
#define RES_ALEAT_MAX 256
#define RES_ANICO_MAX 667
#define RES_ANIMA_MAX 2046
#define RES_ARPLA_MAX 7910
#define RES_CARPC_MAX 384
#define RES_GAUSS_MAX 1449
#define RES_ICONE_MAX 2756
#define RES_LUTIN_MAX 2800
#define RES_MURSM_MAX 76
#define RES_SOUCO_MAX 424
#define RES_SOURI_MAX 1152
#define RES_TEMPL_MAX 27337
#define RES_ZONES_MAX 9014
#define RES_PUZZL_MAX 45671
#define RES_SPRIT_MAX 23811
#define RES_PERS1_MAX 14294
#define RES_PERS2_MAX 10587
#define RES_DESCI_MAX 10515
#define RES_DIALI_MAX 9636
#define RES_MOTSI_MAX 1082
#define RES_VEPCI_MAX 1345
#endif

extern byte vepci_data[];
extern byte motsi_data[];

extern byte puzzl_data[];
#if defined(VERSION_US_EGA)
extern byte puzz1_data[];
#endif
extern byte sprit_data[];

#if defined(VERSION_US_EGA)
extern byte perso_data[];
#else
extern byte pers1_data[];
extern byte pers2_data[];
#endif

extern byte desci_data[];
extern byte diali_data[];

extern byte arpla_data[];
extern byte aleat_data[];
extern byte carpc_data[];
extern byte icone_data[];
extern byte souco_data[];
extern byte souri_data[];
extern byte templ_data[];
extern byte mursm_data[];
extern byte gauss_data[];
extern byte lutin_data[];
extern byte anima_data[];
extern byte anico_data[];
extern byte zones_data[];

byte *SeekToEntry(byte *bank, unsigned int num, byte **end);
byte *SeekToEntryW(byte *bank, unsigned int num, byte **end);

unsigned int LoadFile(const char *filename, byte *buffer);
unsigned int SaveFile(char *filename, byte *buffer, unsigned int size);
int LoadFilesList(ResEntry_t *entries);

int LoadStaticData(void);
int LoadFond(void);
int LoadSpritesData(void);
int LoadPersData(void);
int LoadTextData(void);
int LoadPuzzlData(void);

#endif
