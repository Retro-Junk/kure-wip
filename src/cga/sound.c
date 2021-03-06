#include <dos.h>
#include <conio.h>
#include "common.h"
#include "sound.h"
#include "ifgm.h"

typedef struct pcsample_t {
	unsigned short repeat;
	unsigned short delay1;
	unsigned short delay1sweep;
	unsigned short delay2;
	unsigned short delay2sweep;
	unsigned short freq;
	unsigned short freqsweep;
} pcsample_t;

#define N(x) (0xF000u|x)

pcsample_t pc_samples[] = {
	{ 64,   32,      0,   32,      0,    256,      0},
	{  1, 2560,      0, 2560,      0,      0,      0},
	{  8, 2048,      0,  256,    256,      0,      0},
	{ 11, 2816, N(256), 2816, N(256),    256,    256},
	{  1,  768,      0,  768,      0,    256,      0},
	{128,   48,      1,   48,      0, 61440u,      1},
	{128,   80,      1,   80,      0,      1,      1},
	{128,  128,      1,  128,      1,     16,      1},
	{128,  128,      1,  128,      0,    256,     16},
	{128,  128,      1,  128,      1, 61440u,  N(16)},
	{ 16, 1536,      0, 1536,      0,      0,      0},
	{  3,  768,      0,  768, N(256),    256, N(256)},
};

#undef N

static void SpeakerPlay(pcsample_t *sample) {
	unsigned short rep, freq, delay1, delay2, delay;
	unsigned char ppi;

	freq = sample->freq;
	delay1 = sample->delay1;
	delay2 = sample->delay2;

	disable();
	ppi = inportb(0x61);

	for (rep = 0; rep < sample->repeat; rep++) {
		outportb(0x43, 0xB6);
		outportb(0x42, freq & 255);
		outportb(0x42, freq >> 8);
		/*speaker off*/
		outportb(0x61, ppi & ~3);
		for (delay = delay1; delay--;) ; /*TODO: weak delay*/
		/*speaker on*/
		outportb(0x61, ppi | 3);
		for (delay = delay2; delay--;) ; /*TODO: weak delay*/

		if (sample->delay1sweep & 0xF000)
			delay1 -= sample->delay1sweep & 0xFFF;
		else
			delay1 += sample->delay1sweep;

		if (sample->delay2sweep & 0xF000)
			delay2 -= sample->delay2sweep & 0xFFF;
		else
			delay2 += sample->delay2sweep;

		if (sample->freqsweep & 0xF000)
			freq -= sample->freqsweep & 0xFFF;
		else
			freq += sample->freqsweep;
	}

	/*turn off the speaker*/
	outportb(0x61, ppi & ~3);
	enable();
}

#define kMaxSounds 12

byte sounds_table[kMaxSounds][3] = {
	{20, 0, 0},
	{19, 0, 0},
	{176, 0, 0},
	{144, 145, 146},
	{243, 0, 0},
	{18, 0, 0},
	{149, 21, 0},
	{27, 0, 0},
	{241, 25, 151},
	{22, 0, 0},
	{224, 0, 0},
	{31, 0, 0}
};

void PlaySound(byte index) {
	int i;
	if (IFGM_PlaySound(index))
		return;

	for (i = 0; i < kMaxSounds; i++) {
		if (sounds_table[i][0] == index
		        || sounds_table[i][1] == index
		        || sounds_table[i][2] == index) {
			SpeakerPlay(&pc_samples[i]);
			break;
		}
	}
}
