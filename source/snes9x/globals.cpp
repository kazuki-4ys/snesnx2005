#include "../copyright"

#include "snes9x.h"
#include "memmap.h"
#include "ppu.h"
#include "dsp1.h"
#include "cpuexec.h"
#include "apu.h"
#include "dma.h"
#include "fxemu.h"
#include "gfx.h"
#include "soundux.h"
#include "cheats.h"
#include "sa1.h"
#include "spc7110.h"

char String[513];

SICPU ICPU;
SCPUState CPU;

#ifndef USE_BLARGG_APU
SAPU APU;
SIAPU IAPU;
SSoundData SoundData;
#endif

SSettings Settings;
SDSP1 DSP1;
SSA1 SA1;

SnesModel M1SNES = {1, 3, 2};
SnesModel M2SNES = {2, 4, 3};
SnesModel* Model = &M1SNES;

int32_t OpAddress = 0;

CMemory Memory;

uint8_t OpenBus = 0;

FxInit_s SuperFX;

SPPU PPU;
InternalPPU IPPU;

SDMA DMA[8];

uint8_t* HDMAMemPointers [8];
uint8_t* HDMABasePointers [8];

SBG BG;

SGFX GFX;
SLineData LineData[240];
SLineMatrixData LineMatrixData [240];

uint8_t Mode7Depths [2];
NormalTileRenderer DrawTilePtr = NULL;
ClippedTileRenderer DrawClippedTilePtr = NULL;
NormalTileRenderer DrawHiResTilePtr = NULL;
ClippedTileRenderer DrawHiResClippedTilePtr = NULL;
LargePixelRenderer DrawLargePixelPtr = NULL;

uint32_t odd_high[4][16];
uint32_t odd_low[4][16];
uint32_t even_high[4][16];
uint32_t even_low[4][16];

SCheatData Cheat;

#ifdef LAGFIX
bool finishedFrame = false;
#endif

#ifndef USE_BLARGG_APU
SoundStatus so;

int32_t Echo [24000];
int32_t MixBuffer [SOUND_BUFFER_SIZE];
int32_t EchoBuffer [SOUND_BUFFER_SIZE];
int32_t FilterTaps [8];
uint32_t Z = 0;
int32_t Loop [16];
#endif
uint16_t SignExtend [2] =
{
   0x00, 0xff00
};

/*modified per anomie Mode 5 findings */
int32_t HDMA_ModeByteCounts [8] =
{
   1, 2, 2, 4, 4, 4, 2, 4
};

uint8_t BitShifts[8][4] =
{
   {2, 2, 2, 2}, /* 0 */
   {4, 4, 2, 0}, /* 1 */
   {4, 4, 0, 0}, /* 2 */
   {8, 4, 0, 0}, /* 3 */
   {8, 2, 0, 0}, /* 4 */
   {4, 2, 0, 0}, /* 5 */
   {4, 0, 0, 0}, /* 6 */
   {8, 0, 0, 0}  /* 7 */
};
uint8_t TileShifts[8][4] =
{
   {4, 4, 4, 4}, /* 0 */
   {5, 5, 4, 0}, /* 1 */
   {5, 5, 0, 0}, /* 2 */
   {6, 5, 0, 0}, /* 3 */
   {6, 4, 0, 0}, /* 4 */
   {5, 4, 0, 0}, /* 5 */
   {5, 0, 0, 0}, /* 6 */
   {6, 0, 0, 0}  /* 7 */
};
uint8_t PaletteShifts[8][4] =
{
   {2, 2, 2, 2}, /* 0 */
   {4, 4, 2, 0}, /* 1 */
   {4, 4, 0, 0}, /* 2 */
   {0, 4, 0, 0}, /* 3 */
   {0, 2, 0, 0}, /* 4 */
   {4, 2, 0, 0}, /* 5 */
   {4, 0, 0, 0}, /* 6 */
   {0, 0, 0, 0}  /* 7 */
};
uint8_t PaletteMasks[8][4] =
{
   {7, 7, 7, 7}, /* 0 */
   {7, 7, 7, 0}, /* 1 */
   {7, 7, 0, 0}, /* 2 */
   {0, 7, 0, 0}, /* 3 */
   {0, 7, 0, 0}, /* 4 */
   {7, 7, 0, 0}, /* 5 */
   {7, 0, 0, 0}, /* 6 */
   {0, 0, 0, 0}  /* 7 */
};
uint8_t Depths[8][4] =
{
   {TILE_2BIT, TILE_2BIT, TILE_2BIT, TILE_2BIT}, /* 0 */
   {TILE_4BIT, TILE_4BIT, TILE_2BIT, 0},         /* 1 */
   {TILE_4BIT, TILE_4BIT, 0,         0},         /* 2 */
   {TILE_8BIT, TILE_4BIT, 0,         0},         /* 3 */
   {TILE_8BIT, TILE_2BIT, 0,         0},         /* 4 */
   {TILE_4BIT, TILE_2BIT, 0,         0},         /* 5 */
   {TILE_4BIT, 0,         0,         0},         /* 6 */
   {0,         0,         0,         0}          /* 7 */
};
uint8_t BGSizes [2] =
{
   8, 16
};
uint16_t DirectColourMaps [8][256];

int32_t FilterValues[4][2] =
{
   {0,    0},
   {240,  0},
   {488, -240},
   {460, -208}
};

int32_t NoiseFreq [32] =
{
   0, 16, 21, 25, 31, 42, 50, 63, 84, 100, 125, 167, 200, 250, 333,
   400, 500, 667, 800, 1000, 1300, 1600, 2000, 2700, 3200, 4000,
   5300, 6400, 8000, 10700, 16000, 32000
};

uint32_t HeadMask [4] =
{
#ifdef MSB_FIRST
   0xffffffff, 0x00ffffff, 0x0000ffff, 0x000000ff
#else
   0xffffffff, 0xffffff00, 0xffff0000, 0xff000000
#endif
};

uint32_t TailMask [5] =
{
#ifdef MSB_FIRST
   0x00000000, 0xff000000, 0xffff0000, 0xffffff00, 0xffffffff
#else
   0x00000000, 0x000000ff, 0x0000ffff, 0x00ffffff, 0xffffffff
#endif
};

uint8_t APUROM [64] =
{
   0xCD, 0xEF, 0xBD, 0xE8, 0x00, 0xC6, 0x1D, 0xD0, 0xFC, 0x8F, 0xAA, 0xF4, 0x8F, 0xBB, 0xF5, 0x78,
   0xCC, 0xF4, 0xD0, 0xFB, 0x2F, 0x19, 0xEB, 0xF4, 0xD0, 0xFC, 0x7E, 0xF4, 0xD0, 0x0B, 0xE4, 0xF5,
   0xCB, 0xF4, 0xD7, 0x00, 0xFC, 0xD0, 0xF3, 0xAB, 0x01, 0x10, 0xEF, 0x7E, 0xF4, 0x10, 0xEB, 0xBA,
   0xF6, 0xDA, 0x00, 0xBA, 0xF4, 0xC4, 0xF4, 0xDD, 0x5D, 0xD0, 0xDB, 0x1F, 0x00, 0x00, 0xC0, 0xFF
};

/* Raw SPC700 instruction cycle lengths */
uint8_t S9xAPUCycleLengths [256] =
{
   /*        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, a, b, c, d, e,  f, */
   /* 00 */  2, 8, 4, 5, 3, 4, 3, 6, 2, 6, 5, 4, 5, 4, 6,  8,
   /* 10 */  2, 8, 4, 5, 4, 5, 5, 6, 5, 5, 6, 5, 2, 2, 4,  6,
   /* 20 */  2, 8, 4, 5, 3, 4, 3, 6, 2, 6, 5, 4, 5, 4, 5,  4,
   /* 30 */  2, 8, 4, 5, 4, 5, 5, 6, 5, 5, 6, 5, 2, 2, 3,  8,
   /* 40 */  2, 8, 4, 5, 3, 4, 3, 6, 2, 6, 4, 4, 5, 4, 6,  6,
   /* 50 */  2, 8, 4, 5, 4, 5, 5, 6, 5, 5, 4, 5, 2, 2, 4,  3,
   /* 60 */  2, 8, 4, 5, 3, 4, 3, 6, 2, 6, 4, 4, 5, 4, 5,  5,
   /* 70 */  2, 8, 4, 5, 4, 5, 5, 6, 5, 5, 5, 5, 2, 2, 3,  6,
   /* 80 */  2, 8, 4, 5, 3, 4, 3, 6, 2, 6, 5, 4, 5, 2, 4,  5,
   /* 90 */  2, 8, 4, 5, 4, 5, 5, 6, 5, 5, 5, 5, 2, 2, 12, 5,
   /* a0 */  3, 8, 4, 5, 3, 4, 3, 6, 2, 6, 4, 4, 5, 2, 4,  4,
   /* b0 */  2, 8, 4, 5, 4, 5, 5, 6, 5, 5, 5, 5, 2, 2, 3,  4,
   /* c0 */  3, 8, 4, 5, 4, 5, 4, 7, 2, 5, 6, 4, 5, 2, 4,  9,
   /* d0 */  2, 8, 4, 5, 5, 6, 6, 7, 4, 5, 4, 5, 2, 2, 6,  3,
   /* e0 */  2, 8, 4, 5, 3, 4, 3, 6, 2, 4, 5, 3, 4, 3, 4,  3,
   /* f0 */  2, 8, 4, 5, 4, 5, 5, 6, 3, 4, 5, 4, 2, 2, 4,  3
};

/* Actual data used by CPU emulation, will be scaled by APUReset routine
 * to be relative to the 65c816 instruction lengths. */
uint8_t S9xAPUCycles [256] =
{
   /*        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, a, b, c, d, e,  f, */
   /* 00 */  2, 8, 4, 5, 3, 4, 3, 6, 2, 6, 5, 4, 5, 4, 6,  8,
   /* 10 */  2, 8, 4, 5, 4, 5, 5, 6, 5, 5, 6, 5, 2, 2, 4,  6,
   /* 20 */  2, 8, 4, 5, 3, 4, 3, 6, 2, 6, 5, 4, 5, 4, 5,  4,
   /* 30 */  2, 8, 4, 5, 4, 5, 5, 6, 5, 5, 6, 5, 2, 2, 3,  8,
   /* 40 */  2, 8, 4, 5, 3, 4, 3, 6, 2, 6, 4, 4, 5, 4, 6,  6,
   /* 50 */  2, 8, 4, 5, 4, 5, 5, 6, 5, 5, 4, 5, 2, 2, 4,  3,
   /* 60 */  2, 8, 4, 5, 3, 4, 3, 6, 2, 6, 4, 4, 5, 4, 5,  5,
   /* 70 */  2, 8, 4, 5, 4, 5, 5, 6, 5, 5, 5, 5, 2, 2, 3,  6,
   /* 80 */  2, 8, 4, 5, 3, 4, 3, 6, 2, 6, 5, 4, 5, 2, 4,  5,
   /* 90 */  2, 8, 4, 5, 4, 5, 5, 6, 5, 5, 5, 5, 2, 2, 12, 5,
   /* a0 */  3, 8, 4, 5, 3, 4, 3, 6, 2, 6, 4, 4, 5, 2, 4,  4,
   /* b0 */  2, 8, 4, 5, 4, 5, 5, 6, 5, 5, 5, 5, 2, 2, 3,  4,
   /* c0 */  3, 8, 4, 5, 4, 5, 4, 7, 2, 5, 6, 4, 5, 2, 4,  9,
   /* d0 */  2, 8, 4, 5, 5, 6, 6, 7, 4, 5, 4, 5, 2, 2, 6,  3,
   /* e0 */  2, 8, 4, 5, 3, 4, 3, 6, 2, 4, 5, 3, 4, 3, 4,  3,
   /* f0 */  2, 8, 4, 5, 4, 5, 5, 6, 3, 4, 5, 4, 2, 2, 4,  3
};
