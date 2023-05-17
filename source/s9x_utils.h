#ifndef _S9H_UTILS_H_
#define _S9H_UTILS_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include "utils.h"

#include "snes9x/snes9x.h"
#include "snes9x/cpuexec.h"
#include "snes9x/apu.h"
#include "snes9x/apu_blargg.h"
#include "snes9x/soundux.h"
#include "snes9x/memmap.h"
#include "snes9x/gfx.h"
#include "snes9x/cheats.h"
#include "snes9x/spc7110.h"
#include "snes9x/srtc.h"
#include "snes9x/sa1.h"

void setJoypadInput(unsigned int input);
uint32_t S9xReadJoypad(int32_t port);
bool S9xReadMousePosition(int32_t which1, int32_t* x, int32_t* y, uint32_t* buttons);
bool S9xReadSuperScopePosition(int32_t* x, int32_t* y, uint32_t* buttons);
unsigned int getStateSaveSize(void);
unsigned char *S9xUtilSaveState(void);
bool S9xUtilLoadState(const unsigned char* data, unsigned int size);
bool takeSnapShot(const char *destPath);

#endif//_S9H_UTILS_H_