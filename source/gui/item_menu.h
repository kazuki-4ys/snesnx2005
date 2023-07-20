#ifndef _ITEM_MENU_H_
#define _ITEM_MENU_H_

#define SCREEN_W 1280
#define SCREEN_H 720

#include <string>
#include <vector>

#include <time.h>
#include <sys/time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <switch.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <glad/glad.h>

#define GLM_FORCE_PURE
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <SDL.h>
#include <SDL_mixer.h>

#include "../utils.h"
#include "filemanager.h"
#include "../opengl/mesh.h"
#include "../opengl/text_mesh.h"
#include "../s9x_utils.h"

using namespace std;

#define JOY_A     0
#define JOY_B     1
#define JOY_X     2
#define JOY_Y     3
#define JOY_L     6
#define JOY_R     7
#define JOY_ZL    8
#define JOY_ZR    9
#define JOY_PLUS  10
#define JOY_MINUS 11
#define JOY_LEFT  12
#define JOY_UP    13
#define JOY_RIGHT 14
#define JOY_DOWN  15

#define APP_ITEM_MENU_COUNT 4
#define MAIN_MENU_ITEM_COUNT 6
#define PATH_DISPLAY_COUNT 10
#define STATE_SLOT_COUNT 4

string intToString(unsigned int i);

class ItemMenu;

class App{
        string romPath = "";
        PadState pad;
        EGLDisplay s_display;
        EGLContext s_context;
        EGLSurface s_surface;
        u64 s_startTicks;
        unsigned char *s9xRenderTex = nullptr;
        unsigned char *ttfFont;
        unsigned int ttfFontSize = 0;
        unsigned int stateSaveSize = 0;
        SDL_AudioSpec want, have;
        SDL_AudioDeviceID dev;
        bool initEgl(NWindow* win);
        void deinitEgl(void);
        void run1Fr(float timeDelta);
        float getTime(void);
        void InitSnes9x(void);
        void initSound(void);
        string getBasePath(string path);
        void getSharedFont(void);
        void DeinitSnes9x(void);
        void resetSoundBuffer(void);
        void deinitSound(void);
    public:
        string romPathBase = "";
        bool alreadyLoadROM = false;
        ItemMenu *menu[APP_ITEM_MENU_COUNT];
        int16_t *soundBuffer = nullptr;
        unsigned int soundBufferOutPos = 0;
        unsigned int soundBufferStuckCount = 0;
        unsigned int s9xGeneratedPos = 0;
        bool skipMenuFlag = false;
        bool requestExit = false;
        App(void);
        void loadRom(string path);
        void saveSram(void);
        bool saveState(unsigned int slot);
        bool loadState(unsigned int slot);
        void run(void);
        unsigned char *createS9xRenderTex(void);
        void soundCallBack(uint8_t *_stream, int len);
        void resetS9x(void);
        bool isStateSaveExsist(unsigned int slot);
        ~App(void);
};

class SimpleRect{
        TwoDColorQuad *line[4];
    public:
        SimpleRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, float z);
        void run1Fr(float timeDelta);
        void render(void);
        ~SimpleRect(void);
};

class ItemMenu{
    protected:
        bool renderFlag = false;
        unsigned int itemCount = 1;
        App *pApp;
    public:
        unsigned int curItemindex = 0;
        bool active = false;
        ItemMenu(App *a);
        virtual void run1Fr(unsigned int joyD, unsigned int joyU, unsigned int joyH);
        virtual void render(void);
        virtual ~ItemMenu(void);
};

class S9XRunMenu : public ItemMenu{
        int16_t *mixSamplesBuffer = nullptr;
    protected:
        unsigned int s9xInput = 0;
        TwoDTexQuad *bgLMesh = nullptr;
        TwoDTexQuad *bgRMesh = nullptr;
        TwoDTexQuad *notAlreadyLoadROMBg = nullptr;
        TwoDDynamicTexQuad *s9xRenderer = nullptr;
        void runS9X1Fr(void);
    public:
        S9XRunMenu(App *a);
        void run1Fr(unsigned int joyD, unsigned int joyU, unsigned int joyH);
        void s9XRunMenuRender(void);
        void render(void);
        ~S9XRunMenu(void);
};

class MainMenu : public S9XRunMenu{
    protected:
        unsigned char *ttfFont = nullptr;
        unsigned int ttfFontSize = 0;
        TwoDColorQuad *bokashiDarkQuad = nullptr;
        TwoDColorQuad *whiteQuad = nullptr;
        TwoDColorQuad *VerticalLine = nullptr;
        TextMesh *menuTextMesh[MAIN_MENU_ITEM_COUNT * 2];
    public:
        MainMenu(App *a, unsigned char *ttfFont, unsigned int ttfFontSize);
        void run1Fr(unsigned int joyD, unsigned int joyU, unsigned int joyH);
        void MainMenuRender(void);
        void render(void);
        ~MainMenu(void);
};

class StateSave : public MainMenu{
        TwoDColorQuad *SSwhiteQuad = nullptr;
        TextMesh *slotText[STATE_SLOT_COUNT * 2];
        SimpleRect *slotDisplayRect[STATE_SLOT_COUNT * 2];
        TwoDTexQuad *stateSnap[STATE_SLOT_COUNT];
    public:
        bool isSave = false;//Load or Save
        StateSave(App *a, unsigned char *ttfFont, unsigned int ttfFontSize);
        void prepareForActive(void);
        void run1Fr(unsigned int joyD, unsigned int jpyU, unsigned int joyH);
        void render(void);
        ~StateSave(void);
};

class ROMLoadMenu : public MainMenu{
        int page = 0;
        FileManager *fm = nullptr;
        vector<entry> displayEntries;
        TextMesh *curDirDisplay = nullptr;
        TextMesh *pathDisplay[PATH_DISPLAY_COUNT * 2];
        TwoDColorQuad *RLwhiteQuad = nullptr;
        TwoDColorQuad *horizontalLine = nullptr;
        bool isDir(string fullPath);
        string getExtension(string path);
        bool displayThisPath(string fullPath);
    public:
        ROMLoadMenu(App *a, unsigned char *ttfFont, unsigned int ttfFontSize);
        void fetchPaths(void);
        void updateTextures(void);
        void run1Fr(unsigned int joyD, unsigned int jpyU, unsigned int joyH);
        void render(void);
        ~ROMLoadMenu(void);
};

#endif//_ITEM_MENU_H_