#include "../copyright"

#ifndef _GFX_H_
#define _GFX_H_

#include <stddef.h>

#include "port.h"
#include "ppu.h"
#include "snes9x.h"

void S9xStartScreenRefresh(void);
void S9xDrawScanLine(uint8_t Line);
void S9xEndScreenRefresh(void);
void S9xSetupOBJ(void);
void S9xUpdateScreen(void);
void RenderLine(uint8_t line);
void S9xBuildDirectColourMaps(void);

bool S9xInitGFX(void);
void S9xDeinitGFX(void);

extern bool reduce_sprite_flicker;

typedef struct
{
   uint8_t*    Screen_buffer;
   uint8_t*    SubScreen_buffer;
   uint8_t*    ZBuffer_buffer;
   uint8_t*    SubZBuffer_buffer;
   uint8_t*    Screen;
   uint8_t*    SubScreen;
   uint8_t*    ZBuffer;
   uint8_t*    SubZBuffer;
   uint32_t    Pitch;

   int32_t     Delta;
#if defined(USE_OLD_COLOUR_OPS)
   /* Pre-1.60 colour operations */
   uint16_t*   X2;
   uint16_t*   ZERO_OR_X2;
#endif
   uint16_t*   ZERO;
   uint32_t    RealPitch;  /* True pitch of Screen buffer. */
   uint32_t    Pitch2;     /* Same as RealPitch except while using speed up hack for Glide. */
   uint32_t    ZPitch;     /* Pitch of ZBuffer */
   uint32_t    PPL;        /* Number of pixels on each of Screen buffer */
   uint32_t    PPLx2;
   uint32_t    PixSize;
   uint8_t*    S;
   uint8_t*    DB;
   ptrdiff_t   DepthDelta;
   uint8_t     Z1;         /* Depth for comparison */
   uint8_t     Z2;         /* Depth to save */
   uint32_t    FixedColour;
   uint32_t    StartY;
   uint32_t    EndY;
   ClipData*   pCurrentClip;
   uint32_t    Mode7Mask;
   uint32_t    Mode7PriorityMask;
   uint8_t     OBJWidths[128];
   uint8_t     OBJVisibleTiles[128];

   struct
   {
      uint8_t RTOFlags;
      int16_t Tiles;

      struct
      {
         int8_t  Sprite;
         uint8_t Line;
      } OBJ[32];
   } OBJLines [SNES_HEIGHT_EXTENDED];

   uint8_t     r212c;
   uint8_t     r212d;
   uint8_t     r2130;
   uint8_t     r2131;
   bool        Pseudo;
} SGFX;

/* External port interface which must be implemented or initialised for each port. */
extern SGFX GFX;

typedef struct
{
   struct
   {
      uint16_t VOffset;
      uint16_t HOffset;
   } BG [4];
} SLineData;

#define H_FLIP 0x4000
#define V_FLIP 0x8000
#define BLANK_TILE 2

typedef struct
{
   uint32_t TileSize;
   uint32_t BitShift;
   uint32_t TileShift;
   uint32_t TileAddress;
   uint32_t NameSelect;
   uint32_t SCBase;
   uint32_t StartPalette;
   uint32_t PaletteShift;
   uint32_t PaletteMask;
   uint8_t* Buffer;
   uint8_t* Buffered;
   bool     DirectColourMode;
} SBG;

typedef struct
{
   int16_t MatrixA;
   int16_t MatrixB;
   int16_t MatrixC;
   int16_t MatrixD;
   int16_t CentreX;
   int16_t CentreY;
} SLineMatrixData;

extern uint32_t even_high [4][16];
extern uint32_t even_low  [4][16];
extern uint32_t odd_high  [4][16];
extern uint32_t odd_low   [4][16];
extern SBG BG;
extern uint16_t DirectColourMaps [8][256];

extern uint8_t mul_brightness [16][32];

/* Could use BSWAP instruction on Intel port... */
#define SWAP_DWORD(dword) dword = ((((dword) & 0x000000ff) << 24) \
                                |  (((dword) & 0x0000ff00) <<  8) \
                                |  (((dword) & 0x00ff0000) >>  8) \
                                |  (((dword) & 0xff000000) >> 24))

#ifdef FAST_LSB_WORD_ACCESS
#define READ_2BYTES(s)     (*(uint16_t *) (s))
#define WRITE_2BYTES(s, d)  *(uint16_t *) (s) = (d)
#elif defined(MSB_FIRST)
#define READ_2BYTES(s)     (*(uint8_t *) (s) | (*((uint8_t *) (s) + 1) << 8))
#define WRITE_2BYTES(s, d)  *(uint8_t *) (s) = (d), *((uint8_t *) (s) + 1) = (d) >> 8
#else
#define READ_2BYTES(s)     (*(uint16_t *) (s))
#define WRITE_2BYTES(s, d)  *(uint16_t *) (s) = (d)
#endif

#define SUB_SCREEN_DEPTH 0
#define MAIN_SCREEN_DEPTH 32

static uint16_t COLOR_ADD(uint16_t C1, uint16_t C2)
{
#if defined(USE_OLD_COLOUR_OPS)
   /* Pre-1.60 colour operations */
   if (C1 == 0)
      return C2;
   else if (C2 == 0)
      return C1;
   else
      return GFX.X2[(((C1 & RGB_REMOVE_LOW_BITS_MASK) + (C2 & RGB_REMOVE_LOW_BITS_MASK)) >> 1) + (C1 & C2 & RGB_LOW_BITS_MASK)] | ((C1 ^ C2) & RGB_LOW_BITS_MASK);
#else
	const int RED_MASK   = 0x1F << RED_SHIFT_BITS;
	const int GREEN_MASK = 0x1F << GREEN_SHIFT_BITS;
	const int BLUE_MASK  = 0x1F;

	int rb          = (C1 & (RED_MASK | BLUE_MASK)) + (C2 & (RED_MASK | BLUE_MASK));
	int rbcarry     = rb & ((0x20 << RED_SHIFT_BITS) | (0x20 << 0));
	int g           = (C1 & (GREEN_MASK)) + (C2 & (GREEN_MASK));
	int rgbsaturate = (((g & (0x20 << GREEN_SHIFT_BITS)) | rbcarry) >> 5) * 0x1f;
	uint16_t retval = (rb & (RED_MASK | BLUE_MASK)) | (g & GREEN_MASK) | rgbsaturate;

#if GREEN_SHIFT_BITS == 6
	retval         |= (retval & 0x0400) >> 5;
#endif

	return retval;
#endif
}

#define COLOR_ADD1_2(C1, C2) \
(((((C1) & RGB_REMOVE_LOW_BITS_MASK) + \
          ((C2) & RGB_REMOVE_LOW_BITS_MASK)) >> 1) + \
         (((C1) & (C2) & RGB_LOW_BITS_MASK) | ALPHA_BITS_MASK))

#if defined(USE_OLD_COLOUR_OPS)
/* Pre-1.60 colour operations */
#define COLOR_SUB(C1, C2) \
(GFX.ZERO_OR_X2 [(((C1) | RGB_HI_BITS_MASKx2) - \
                  ((C2) & RGB_REMOVE_LOW_BITS_MASK)) >> 1] + \
                  ((C1) & RGB_LOW_BITS_MASK) - \
                  ((C2) & RGB_LOW_BITS_MASK))
#else
static uint16_t COLOR_SUB(uint16_t C1, uint16_t C2)
{
	int rb1         = (C1 & (THIRD_COLOR_MASK | FIRST_COLOR_MASK)) | ((0x20 << 0) | (0x20 << RED_SHIFT_BITS));
	int rb2         = C2 & (THIRD_COLOR_MASK | FIRST_COLOR_MASK);
	int rb          = rb1 - rb2;
	int rbcarry     = rb & ((0x20 << RED_SHIFT_BITS) | (0x20 << 0));
	int g           = ((C1 & (SECOND_COLOR_MASK)) | (0x20 << GREEN_SHIFT_BITS)) - (C2 & (SECOND_COLOR_MASK));
	int rgbsaturate = (((g & (0x20 << GREEN_SHIFT_BITS)) | rbcarry) >> 5) * 0x1f;
	uint16_t retval = ((rb & (THIRD_COLOR_MASK | FIRST_COLOR_MASK)) | (g & SECOND_COLOR_MASK)) & rgbsaturate;

#if GREEN_SHIFT_BITS == 6
	retval         |= (retval & 0x0400) >> 5;
#endif

	return retval;
}
#endif

#define COLOR_SUB1_2(C1, C2) \
GFX.ZERO [(((C1) | RGB_HI_BITS_MASKx2) - \
           ((C2) & RGB_REMOVE_LOW_BITS_MASK)) >> 1]

typedef void (*NormalTileRenderer)(uint32_t Tile, int32_t Offset, uint32_t StartLine, uint32_t LineCount);
typedef void (*ClippedTileRenderer)(uint32_t Tile, int32_t Offset, uint32_t StartPixel, uint32_t Width, uint32_t StartLine, uint32_t LineCount);
typedef void (*LargePixelRenderer)(uint32_t Tile, int32_t Offset, uint32_t StartPixel, uint32_t Pixels, uint32_t StartLine, uint32_t LineCount);
#endif
