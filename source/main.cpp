#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <switch.h>

#include <EGL/egl.h>    // EGL library
#include <EGL/eglext.h> // EGL extensions
#include <glad/glad.h>  // glad library (OpenGL loader)

// GLM headers
#define GLM_FORCE_PURE
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>

#include "gui/item_menu.h"
#include "s9x_utils.h"
#include "snes9x/display.h"
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
#include "opengl/shader.h"
#include "opengl/text_mesh.h"
#include "utils.h"

using namespace std;

FT_Library ftLibrary;
App *a;

//-----------------------------------------------------------------------------
// nxlink support
//-----------------------------------------------------------------------------

#ifndef ENABLE_NXLINK
#define TRACE(fmt,...) ((void)0)
#else
#include <unistd.h>
#define TRACE(fmt,...) printf("%s: " fmt "\n", __PRETTY_FUNCTION__, ## __VA_ARGS__)

static int s_nxlinkSock = -1;

static void initNxLink()
{
    if (R_FAILED(socketInitializeDefault()))
        return;

    s_nxlinkSock = nxlinkStdio();
    if (s_nxlinkSock >= 0)
        TRACE("printf output now goes to nxlink server");
    else
        socketExit();
}

static void deinitNxLink()
{
    if (s_nxlinkSock >= 0)
    {
        close(s_nxlinkSock);
        socketExit();
        s_nxlinkSock = -1;
    }
}

extern "C" void userAppInit()
{
    initNxLink();
}

extern "C" void userAppExit()
{
    deinitNxLink();
}

#endif

unsigned int convertPadButtonToMyInput(u64 button){
    unsigned int dest = 0;
    if(button & HidNpadButton_Right)dest |= (1 << JOY_RIGHT);
    if(button & HidNpadButton_Left)dest |= (1 << JOY_LEFT);
    if(button & HidNpadButton_Down)dest |= (1 << JOY_DOWN);
    if(button & HidNpadButton_Up)dest |= (1 << JOY_UP);
    if(button & HidNpadButton_A)dest |= (1 << JOY_A);
    if(button & HidNpadButton_B)dest |= (1 << JOY_B);
    if(button & HidNpadButton_X)dest |= (1 << JOY_X);
    if(button & HidNpadButton_Y)dest |= (1 << JOY_Y);
    if(button & HidNpadButton_L)dest |= (1 << JOY_L);
    if(button & HidNpadButton_R)dest |= (1 << JOY_R);
    if(button & HidNpadButton_ZL)dest |= (1 << JOY_ZL);
    if(button & HidNpadButton_ZR)dest |= (1 << JOY_ZR);
    if(button & HidNpadButton_Plus)dest |= (1 << JOY_PLUS);
    if(button & HidNpadButton_Minus)dest |= (1 << JOY_MINUS);
    return dest;
}

App::App(void){
    romfsInit();
    FT_Init_FreeType(&ftLibrary);
    getSharedFont();
    //Open GL初期化
    initEgl(nwindowGetDefault());
    gladLoadGL();

    InitSnes9x();
    initSound();

    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    padInitializeDefault(&pad);

    s_startTicks = armGetSystemTick();

    menu[0] = new S9XRunMenu(this);
    menu[0]->active = true;
    menu[1] = new MainMenu(this, ttfFont, ttfFontSize);
    menu[2] = new StateSave(this, ttfFont, ttfFontSize);
    menu[3] = new ROMLoadMenu(this, ttfFont, ttfFontSize);
}

float App::getTime(){
    u64 elapsed = armGetSystemTick() - s_startTicks;
    return (elapsed * 625 / 12) / 1000000000.0;
}

string App::getBasePath(string path){
    const char *c_str = path.c_str();
    int curIndex = strlen(c_str);
    char *destCstr = (char*)calloc(curIndex + 1, sizeof(char));
    curIndex--;
    while(true){
        if(curIndex < 0)return path;
        if(c_str[curIndex] == '.'){
            memcpy(destCstr, c_str, curIndex);
            string dest = destCstr;
            free(destCstr);
            return dest;
        }
        if(c_str[curIndex] == '/')return path;
        curIndex--;
    }
    return "";
}

void App::InitSnes9x(void){
    memset(&Settings, 0, sizeof(Settings));
    Settings.JoystickEnabled = false;
    Settings.SoundPlaybackRate = 36000;
    //Settings.SoundPlaybackRate = sampleRate;
    Settings.CyclesPercentage = 100;
    Settings.DisableSoundEcho = false;
    Settings.InterpolatedSound = true;
    Settings.APUEnabled = true;
    Settings.H_Max = SNES_CYCLES_PER_SCANLINE;
    Settings.FrameTimePAL = 20000;
    Settings.FrameTimeNTSC = 16667;
    Settings.DisableMasterVolume = false;
    Settings.Mouse = true;
    Settings.SuperScope = true;
    Settings.MultiPlayer5 = true;
    Settings.ControllerOption = SNES_JOYPAD;
    Settings.SoundSync = false;
    Settings.ApplyCheats = true;
    Settings.HBlankStart = (256 * Settings.H_Max) / SNES_HCOUNTER_MAX;
    S9xInitMemory();
    S9xInitAPU();
    S9xInitSound();
    S9xSetPlaybackRate(36000);
    //S9xSetPlaybackRate(sampleRate);
    S9xInitDisplay();
    S9xInitGFX();
    stateSaveSize = getStateSaveSize();
    s9xRenderTex = (unsigned char*)calloc(256 * 224 * 4, sizeof(unsigned char));
}

void SDLSoundCallBack(void *userdata, uint8_t *_stream, int len){
    ((App*)userdata)->soundCallBack(_stream, len);
}

bool App::isStateSaveExsist(unsigned int slot){
    if(!alreadyLoadROM)return false;
    FILE *f = getFileSizeAndPointer((romPathBase + "_slot" + intToString(slot) + ".s95ws").c_str(), nullptr);
    if(f){
        fclose(f);
        return true;
    }
    return false;
}

void App::resetSoundBuffer(void){
    soundBufferOutPos = 0;
    s9xGeneratedPos = 0;
    memset(soundBuffer, 0, 4096 * 2 * sizeof(int16_t));
    soundBufferStuckCount = 0;
}

void App::soundCallBack(uint8_t *_stream, int len){
    if(!soundBuffer)soundBuffer = (int16_t*)calloc(sizeof(int16_t), 2048 * 2 * 2);
    if(!(alreadyLoadROM && menu[0]->active)){
        memset(_stream, 0, 2048 * 2 * 2 * 2);
        return;
    }
    if(soundBufferStuckCount >= 5){//応急処置
        resetSoundBuffer();
    }
    soundBufferStuckCount++;
    if(soundBufferOutPos < 2048){
        if(s9xGeneratedPos < 2048)return;//soundCallBackが呼ばれすぎてS9xSoundCallbackによって生成された音声データに追いついた
    }else{
        if(s9xGeneratedPos >= 2048)return;//soundCallBackが呼ばれすぎてS9xSoundCallbackによって生成された音声データに追いついた
    }
    for(unsigned int i = 0;i < 2048;i++){
        for(unsigned int j = 0;j < 2;j++)((int16_t*)_stream)[i * 2 + j] = soundBuffer[(soundBufferOutPos * 2 + i * 2 + j) % 8192];
    }
    soundBufferOutPos = (soundBufferOutPos + 2048) % 4096;
    soundBufferStuckCount = 0;
    return;
}

void App::resetS9x(void){
    S9xReset();
}

void App::initSound(void){
    SDL_Init(SDL_INIT_AUDIO);
    const char *audioDeviceName = SDL_GetAudioDeviceName(0, 0);
    memset(&want, 0, sizeof(want));
    want.freq = 36000;
    want.format = AUDIO_S16SYS;
    want.channels = 2;
    want.samples = 2048;
    want.callback = SDLSoundCallBack;
    want.userdata = this;
    dev = SDL_OpenAudioDevice(audioDeviceName, 0, &want, &have, 0);
    if(dev == 0)exit(0);
    SDL_PauseAudioDevice(dev, 0);
}

void App::loadRom(string path){
    unsigned int romSize, sramSize;
    unsigned char *rom = fileToBytes(path.c_str(), &romSize);
    if(!rom)return;//pathが見つからない
    romPath = path;
    romPathBase = getBasePath(romPath);
    //snes9xにROMを読ませる
    LoadROMFromBuffer(rom, romSize);
    free(rom);
    alreadyLoadROM = true;
    unsigned char *sram = fileToBytes((romPathBase + ".srm").c_str(), &sramSize);
    if(sram){
        if(sramSize > 0x20000){
            memcpy(Memory.SRAM, sram, 0x20000);
        }else{
            memcpy(Memory.SRAM, sram, sramSize);
        }
        free(sram);
    }
    S9xReset();
}

unsigned char *App::createS9xRenderTex(void){
    unsigned short col;
    for(unsigned int y = 0;y < 224;y++){
        for(unsigned int x = 0;x < 256;x++){
            memcpy(&col, GFX.Screen + 2 * (512 * y + x), 2);
            unsigned char *destPixel = s9xRenderTex + 4 * ((223 - y) * 256 + x);
            *(destPixel + 0) = ((col >> 11) & 0x1F) << 3;
            *(destPixel + 1) = ((col >> 5) & 0x3F) << 2;
            *(destPixel + 2) = ((col >> 0) & 0x1F) << 3;
            *(destPixel + 3) = 0xFF;
        }
    }
    return s9xRenderTex;
}

bool App::initEgl(NWindow* win)
{
    // Connect to the EGL default display
    s_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (!s_display)
    {
        TRACE("Could not connect to display! error: %d", eglGetError());
        goto _fail0;
    }

    // Initialize the EGL display connection
    eglInitialize(s_display, nullptr, nullptr);

    // Select OpenGL (Core) as the desired graphics API
    if (eglBindAPI(EGL_OPENGL_API) == EGL_FALSE)
    {
        TRACE("Could not set API! error: %d", eglGetError());
        goto _fail1;
    }

    // Get an appropriate EGL framebuffer configuration
    EGLConfig config;
    EGLint numConfigs;
    static const EGLint framebufferAttributeList[] =
    {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_RED_SIZE,     8,
        EGL_GREEN_SIZE,   8,
        EGL_BLUE_SIZE,    8,
        EGL_ALPHA_SIZE,   8,
        EGL_DEPTH_SIZE,   24,
        EGL_STENCIL_SIZE, 8,
        EGL_NONE
    };
    eglChooseConfig(s_display, framebufferAttributeList, &config, 1, &numConfigs);
    if (numConfigs == 0)
    {
        TRACE("No config found! error: %d", eglGetError());
        goto _fail1;
    }

    // Create an EGL window surface
    s_surface = eglCreateWindowSurface(s_display, config, win, nullptr);
    if (!s_surface)
    {
        TRACE("Surface creation failed! error: %d", eglGetError());
        goto _fail1;
    }

    // Create an EGL rendering context
    static const EGLint contextAttributeList[] =
    {
        EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR,
        EGL_CONTEXT_MAJOR_VERSION_KHR, 4,
        EGL_CONTEXT_MINOR_VERSION_KHR, 3,
        EGL_NONE
    };
    s_context = eglCreateContext(s_display, config, EGL_NO_CONTEXT, contextAttributeList);
    if (!s_context)
    {
        TRACE("Context creation failed! error: %d", eglGetError());
        goto _fail2;
    }

    // Connect the context to the surface
    eglMakeCurrent(s_display, s_surface, s_surface, s_context);
    return true;

_fail2:
    eglDestroySurface(s_display, s_surface);
    s_surface = nullptr;
_fail1:
    eglTerminate(s_display);
    s_display = nullptr;
_fail0:
    return false;
}

void App::saveSram(void){
    unsigned int sramSize = (1 << Memory.SRAMSize) * 1024;
    bytesToFile(Memory.SRAM, sramSize, (romPathBase + ".srm").c_str());
}

bool App::saveState(unsigned int slot){
    unsigned char *state = S9xUtilSaveState();
    bool result = bytesToFile(state, stateSaveSize, (romPathBase + "_slot" + intToString(slot) + ".s95ws").c_str());
    free(state);
    takeSnapShot((romPathBase + "_slot" + intToString(slot) + ".bmp").c_str());
    return result;
}

bool App::loadState(unsigned int slot){
    unsigned int stateSize;
    unsigned char *state = fileToBytes((romPathBase + "_slot" + intToString(slot) + ".s95ws").c_str(), &stateSize);
    if(!state)return false;
    bool result = S9xUtilLoadState(state, stateSize);
    if(result)saveSram();
    free(state);
    return result;
}

void App::run(void){
    while(appletMainLoop()){
        padUpdate(&pad);
        run1Fr(getTime());
        if(requestExit)return;
    }
}

void App::run1Fr(float timeDelta){
    //フレームバッファのクリア?
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    unsigned int joyD, joyU, joyH;
    joyH = convertPadButtonToMyInput(padGetButtons(&pad));
    joyD = convertPadButtonToMyInput(padGetButtonsDown(&pad));
    joyU = convertPadButtonToMyInput(padGetButtonsUp(&pad));

    skipMenuFlag = false;
    for(unsigned int i = 0;i < APP_ITEM_MENU_COUNT;i++){
        if(skipMenuFlag)break;
        menu[i]->run1Fr(joyD, joyU, joyH);
        menu[i]->render();
    }

    eglSwapBuffers(s_display, s_surface);//フレームバッファのスワップ
}

void App::deinitEgl(void)
{
    if (s_display)
    {
        eglMakeCurrent(s_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (s_context)
        {
            eglDestroyContext(s_display, s_context);
            s_context = nullptr;
        }
        if (s_surface)
        {
            eglDestroySurface(s_display, s_surface);
            s_surface = nullptr;
        }
        eglTerminate(s_display);
        s_display = nullptr;
    }
}

void App::getSharedFont(void){
    ttfFont = nullptr;
    PlFontData plFont;
    Result rc = plInitialize(PlServiceType_User);
    if(R_FAILED(rc))return;
    rc = plGetSharedFontByType(&plFont, PlSharedFontType_Standard);
    if(R_FAILED(rc)){
        plExit();
        return;
    }
    ttfFont = (unsigned char*)plFont.address;
    ttfFontSize = plFont.size;
}

void App::DeinitSnes9x(void){
    free(s9xRenderTex);
    S9xDeinitGFX();
    //S9xDeinitDisplay();
    S9xDeinitAPU();
    S9xDeinitMemory();
    return;
}

void App::deinitSound(void){
    SDL_CloseAudioDevice(dev);
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

App::~App(void){
    for(unsigned int i = 0;i < APP_ITEM_MENU_COUNT;i++)delete menu[i];
    deinitSound();
    DeinitSnes9x();
    deinitEgl();
    if(ttfFont)plExit();
    romfsExit();
}

int main(int argc, char **argv){
    a = new App();
    a->run();
    delete a;
    return 0;
}