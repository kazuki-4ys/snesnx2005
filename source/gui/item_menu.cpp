#include "item_menu.h"

string intToString(unsigned int i){
    char dest[32];
    memset(dest, 0, 32);
    sprintf(dest, "%d", i);
    return dest;
}

void setTextMeshScale(TextMesh *t){
    t->scaleX = (float)t->realTexWidth / SCREEN_W;
    t->scaleY = (float)t->realTexHeight / SCREEN_H;
}

void setTextMeshPos(TextMesh *t, unsigned int x, unsigned int y){
    t->posX = ((x / (float)SCREEN_W) + (t->realTexWidth / (float)SCREEN_W) / 2) * 2.0f - 1.0f;
    t->posY = (((y / (float)SCREEN_H) + (t->realTexHeight / (float)SCREEN_H) / 2) * -2.0f) + 1.0f;
}

void setTwoDColorQuadScale(TwoDTexQuad *t, unsigned int x, unsigned int y){
    t->scaleX = (float)x / SCREEN_W;
    t->scaleY = (float)y / SCREEN_H;
}

void setTwoDColorQuadPosAndScale(TwoDTexQuad *t, int x, int y, unsigned int w, unsigned int h){
    t->scaleX = (float)w / SCREEN_W;
    t->scaleY = (float)h / SCREEN_H;
    t->posX = ((x / (float)SCREEN_W) + (w / (float)SCREEN_W) / 2) * 2.0f - 1.0f;
    t->posY = (((y / (float)SCREEN_H) + (h / (float)SCREEN_H) / 2) * -2.0f) + 1.0f;
}

SimpleRect::SimpleRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, float z){
    for(unsigned int i = 0;i < 4;i++){
        line[i] = new TwoDColorQuad(0xFFFFFFFF);
        line[i]->posZ = z;
    }
    setTwoDColorQuadPosAndScale(line[0], x, y, 1, h);//縦線左
    setTwoDColorQuadPosAndScale(line[1], x + w - 1, y, 1, h);//縦線右
    setTwoDColorQuadPosAndScale(line[2], x, y, w, 1);//横線上
    setTwoDColorQuadPosAndScale(line[3], x, y + h - 1, w, 1);//横線下
}

void SimpleRect::run1Fr(float timeDelta){
    for(unsigned int i = 0;i < 4;i++)line[i]->run1Fr(timeDelta);
}

void SimpleRect::render(void){
    for(unsigned int i = 0;i < 4;i++)line[i]->render();
}

SimpleRect::~SimpleRect(void){
    for(unsigned int i = 0;i < 4;i++)delete line[i];
}

ItemMenu::ItemMenu(App *a){
    pApp = a;
}

void ItemMenu::run1Fr(unsigned int joyD, unsigned int joyU, unsigned int joyH){
    return;
}

void ItemMenu::render(void){
    return;
}

ItemMenu::~ItemMenu(void){
    return;
}

S9XRunMenu::S9XRunMenu(App *a) : ItemMenu(a){
    bgLMesh = new TwoDTexQuad("romfs:/texture/bg_l.png");
    setTwoDColorQuadPosAndScale(bgLMesh, -41, 0, 727, 727);
    bgLMesh->posZ = 0.11f;
    bgRMesh = new TwoDTexQuad("romfs:/texture/bg_r.png");
    setTwoDColorQuadPosAndScale(bgRMesh, 594, 0, 727, 727);
    bgRMesh->posZ = 0.11f;
    notAlreadyLoadROMBg = new TwoDTexQuad("romfs:/texture/snesnx2005.png");
    s9xRenderer = new TwoDDynamicTexQuad();
    setTwoDColorQuadPosAndScale(s9xRenderer, 201, 0, 877, 678);
    notAlreadyLoadROMBg->posZ = 0.1f;
    s9xRenderer->posZ = 0.1f;
}

void S9XRunMenu::runS9X1Fr(void){
    CPU.SRAMModified = false;
    setJoypadInput(s9xInput);
    S9xMainLoop();
    S9xUpdateScreen();
    //S9xMixSamples(pApp->soundBuffer, 1200);
    if(!pApp->soundBuffer)pApp->soundBuffer = (int16_t*)calloc(4096 * 2, sizeof(int16_t));
    if(!mixSamplesBuffer)mixSamplesBuffer = (int16_t*)calloc(600 * 2, sizeof(int16_t));
    S9xMixSamples(mixSamplesBuffer, 600 * 2);
    unsigned int nextPos = (pApp->s9xGeneratedPos + 600) % 4096;
    if(pApp->soundBufferOutPos >= 2048){
        if(pApp->s9xGeneratedPos <= 2048 && nextPos >= 2048)return;
    }else{
        if(pApp->s9xGeneratedPos > 2048 && nextPos <= 2048)return;
    }
    for(unsigned int i = 0;i < 600 * 2; i++){
        pApp->soundBuffer[(pApp->s9xGeneratedPos * 2 + i) % 8192] = mixSamplesBuffer[i];
    }
    pApp->s9xGeneratedPos = (pApp->s9xGeneratedPos + 600) % 4096;
    unsigned short col;
    if(pApp->alreadyLoadROM && CPU.SRAMModified)pApp->saveSram();
    if(pApp->alreadyLoadROM)s9xRenderer->setTex(pApp->createS9xRenderTex(), 256, 224);
}

void S9XRunMenu::run1Fr(unsigned int joyD, unsigned int joyU, unsigned int joyH){
    renderFlag = false;
    if(!active){
        s9xInput = 0;
        return;
    }
    renderFlag = true;

    if((joyH & ((1 << JOY_ZL) | (1 << JOY_ZR))) == ((1 << JOY_ZL) | (1 << JOY_ZR))){//ZLとZR同時押しでメインメニューへ
        active = false;
        pApp->menu[1]->curItemindex = 0;
        pApp->menu[1]->active = true;
        pApp->skipMenuFlag = true;
        return;
    }

    if(!pApp->alreadyLoadROM){
        notAlreadyLoadROMBg->run1Fr(0.0f);
        return;
    }
    if(joyD & (1 << JOY_RIGHT))s9xInput |= (1 << 8);//→
    if(joyD & (1 << JOY_LEFT))s9xInput |= (1 << 9);//←
    if(joyD & (1 << JOY_DOWN))s9xInput |= (1 << 10);//↓
    if(joyD & (1 << JOY_UP))s9xInput |= (1 << 11);//↑
    if(joyD & (1 << JOY_A))s9xInput |= (1 << 7);//A
    if(joyD & (1 << JOY_B))s9xInput |= (1 << 15);//B
    if(joyD & (1 << JOY_X))s9xInput |= (1 << 6);//X
    if(joyD & (1 << JOY_Y))s9xInput |= (1 << 14);//Y
    if(joyD & (1 << JOY_L))s9xInput |= (1 << 5);//L
    if(joyD & (1 << JOY_R))s9xInput |= (1 << 4);//R
    if(joyD & (1 << JOY_MINUS))s9xInput |= (1 << 13);//START
    if(joyD & (1 << JOY_PLUS))s9xInput |= (1 << 12);//SELECT

    if(joyU & (1 << JOY_RIGHT))s9xInput &= (0xFFFFFFFF ^ (1 << 8));
    if(joyU & (1 << JOY_LEFT))s9xInput &= (0xFFFFFFFF ^ (1 << 9));
    if(joyU & (1 << JOY_DOWN))s9xInput &= (0xFFFFFFFF ^ (1 << 10));
    if(joyU & (1 << JOY_UP))s9xInput &= (0xFFFFFFFF ^ (1 << 11));
    if(joyU & (1 << JOY_A))s9xInput &= (0xFFFFFFFF ^ (1 << 7));
    if(joyU & (1 << JOY_B))s9xInput &= (0xFFFFFFFF ^ (1 << 15));
    if(joyU & (1 << JOY_X))s9xInput &= (0xFFFFFFFF ^ (1 << 6));
    if(joyU & (1 << JOY_Y))s9xInput &= (0xFFFFFFFF ^ (1 << 14));
    if(joyU & (1 << JOY_L))s9xInput &= (0xFFFFFFFF ^ (1 << 5));
    if(joyU & (1 << JOY_R))s9xInput &= (0xFFFFFFFF ^ (1 << 4));
    if(joyU & (1 << JOY_MINUS))s9xInput &= (0xFFFFFFFF ^ (1 << 13));
    if(joyU & (1 << JOY_PLUS))s9xInput &= (0xFFFFFFFF ^ (1 << 12));

    runS9X1Fr();
    bgLMesh->run1Fr(0.0f);
    bgRMesh->run1Fr(0.0f);
    s9xRenderer->run1Fr(0.0f);
}

void S9XRunMenu::render(void){
    if(!renderFlag)return;
    s9XRunMenuRender();
}

void S9XRunMenu::s9XRunMenuRender(void){
    if(!pApp->alreadyLoadROM){
        notAlreadyLoadROMBg->render();
        return;
    }
    bgLMesh->render();
    bgRMesh->render();
    s9xRenderer->render();
}

S9XRunMenu::~S9XRunMenu(void){
    delete notAlreadyLoadROMBg;
    delete bgLMesh;
    delete bgRMesh;
    delete s9xRenderer;
}

MainMenu::MainMenu(App *a, unsigned char *aTtfFont, unsigned int aTtfFontSize) : S9XRunMenu(a){
    itemCount = MAIN_MENU_ITEM_COUNT;
    ttfFont = aTtfFont;
    ttfFontSize = aTtfFontSize;
    bokashiDarkQuad = new TwoDColorQuad(0x000000B0);
    bokashiDarkQuad->posZ = 0.09f;
    whiteQuad = new TwoDColorQuad(0xffffffff);
    whiteQuad->posZ = 0.08f;
    setTwoDColorQuadScale(whiteQuad, 492, 48);
    whiteQuad->posX = ((160 / (float)SCREEN_W) + (492 / (float)SCREEN_W) / 2) * 2.0f - 1.0f;
    VerticalLine = new TwoDColorQuad(0xffffffff);
    VerticalLine->posZ = 0.08f;
    setTwoDColorQuadScale(VerticalLine, 1, 666);
    VerticalLine->posX = ((652 / (float)SCREEN_W) + (1 / (float)SCREEN_W) / 2) * 2.0f - 1.0f;
    VerticalLine->posY = (((27 / (float)SCREEN_H) + (666 / (float)SCREEN_H) / 2) * -2.0f) + 1.0f;
    string menuStrings[] = {"Resume", "Load state", "Save state", "Reset", "Select ROM", "Exit"};
    if(ttfFont){
        for(unsigned int i = 0;i < MAIN_MENU_ITEM_COUNT;i++){
            menuTextMesh[i * 2 + 1] = new TextMesh(ttfFont, ttfFontSize, 25, menuStrings[i], 0xFF0000FF);
            menuTextMesh[i * 2] = new TextMesh(ttfFont, ttfFontSize, 25, menuStrings[i], 0xFFFFFFFF);
        }
        for(unsigned int i = 0;i < MAIN_MENU_ITEM_COUNT * 2;i++){
            menuTextMesh[i]->posZ = 0.07f;
            setTextMeshScale(menuTextMesh[i]);
        }
        unsigned int meshTextPosX, meshTextPosY;
        for(unsigned int i = 0;i < MAIN_MENU_ITEM_COUNT;i++){
            meshTextPosX = 652 - (menuTextMesh[i * 2]->realTexWidth + 60);
            meshTextPosY = i * 48 + 165;
            meshTextPosY += (48 - (int)menuTextMesh[i * 2]->realTexHeight) / 2;
            setTextMeshPos(menuTextMesh[i * 2], meshTextPosX, meshTextPosY);
            setTextMeshPos(menuTextMesh[i * 2 + 1], meshTextPosX, meshTextPosY);
        }
    }else{
        for(unsigned int i = 0;i < MAIN_MENU_ITEM_COUNT * 2;i++)menuTextMesh[i] = nullptr;
    }
}

void MainMenu::run1Fr(unsigned int joyD, unsigned int joyU, unsigned int joyH){
    renderFlag = false;
    if(!active)return;
    renderFlag = true;
    if((joyD & (1 << JOY_A)) != 0){//A ボタン押された
        if(curItemindex == 0){
            active = false;
            pApp->menu[0]->active = true;
            pApp->skipMenuFlag = true;
            return;
        }
        if(curItemindex == 1){
            pApp->menu[2]->active = true;
            ((StateSave*)(pApp->menu[2]))->isSave = false;
            ((StateSave*)(pApp->menu[2]))->prepareForActive();
            active = false;
            pApp->skipMenuFlag = true;
            return;
        }
        if(curItemindex == 2){
            pApp->menu[2]->active = true;
            ((StateSave*)(pApp->menu[2]))->isSave = true;
            ((StateSave*)(pApp->menu[2]))->prepareForActive();
            active = false;
            pApp->skipMenuFlag = true;
            return;
        }
        if(curItemindex == 3){
            active = false;
            pApp->menu[0]->active = true;
            pApp->skipMenuFlag = true;
            pApp->resetS9x();
            return;
        }
        if(curItemindex == 4){
            pApp->menu[3]->active = true;
            ((ROMLoadMenu*)(pApp->menu[3]))->fetchPaths();
            ((ROMLoadMenu*)(pApp->menu[3]))->updateTextures();
            active = false;
            pApp->skipMenuFlag = true;
            return;
        }
        if(curItemindex == 5){
            active = false;
            pApp->menu[0]->active = true;
            pApp->skipMenuFlag = true;
            pApp->requestExit = true;
            return;
        }
    }
    if((joyD & (1 << JOY_B)) != 0){//B ボタン押された
        active = false;
        pApp->menu[0]->active = true;
        pApp->skipMenuFlag = true;
        return;
    }
    if((joyD & (1 << JOY_DOWN)) != 0){//↓ ボタン押された
        if(curItemindex < (itemCount - 1))curItemindex++;
    }
    if((joyD & (1 << JOY_UP)) != 0){//↑ ボタン押された
        if(curItemindex > 0)curItemindex--;
    }
    notAlreadyLoadROMBg->run1Fr(0.0f);
    bgLMesh->run1Fr(0.0f);
    bgRMesh->run1Fr(0.0f);
    s9xRenderer->run1Fr(0.0f);
    bokashiDarkQuad->run1Fr(0.0f);
    whiteQuad->posY = ((((curItemindex * 48 + 165) / (float)SCREEN_H) + (48 / (float)SCREEN_H) / 2) * -2.0f) + 1.0f;
    whiteQuad->run1Fr(0.0f);
    VerticalLine->run1Fr(0.0f);
    for(unsigned int i = 0;i < MAIN_MENU_ITEM_COUNT * 2;i++)if(menuTextMesh[i])menuTextMesh[i]->run1Fr(0.0f);
}

void MainMenu::MainMenuRender(void){
    if(pApp->alreadyLoadROM)s9xRenderer->setTex(pApp->createS9xRenderTex(), 256, 224);
    s9XRunMenuRender();
    bokashiDarkQuad->render();
    whiteQuad->render();
    VerticalLine->render();
    for(unsigned int i = 0;i < MAIN_MENU_ITEM_COUNT;i++){
        if(curItemindex == i){
            if(menuTextMesh[i * 2 + 1])menuTextMesh[i * 2 + 1]->render();
        }else{
            if(menuTextMesh[i * 2])menuTextMesh[i * 2]->render();
        }
    }
}

void MainMenu::render(void){
    if(!renderFlag)return;
    MainMenuRender();
}

MainMenu::~MainMenu(void){
    delete bokashiDarkQuad;
    delete whiteQuad;
    delete VerticalLine;
    for(unsigned int i = 0;i < MAIN_MENU_ITEM_COUNT * 2;i++)if(menuTextMesh[i])delete menuTextMesh[i];
}

StateSave::StateSave(App *a, unsigned char *aTtfFont, unsigned int aTtfFontSize) : MainMenu(a, aTtfFont, aTtfFontSize){
    itemCount = STATE_SLOT_COUNT;
    curItemindex = 0;
    for(unsigned int i = 0;i < STATE_SLOT_COUNT;i++)stateSnap[i] = nullptr;
    for(unsigned int i = 0;i < MAIN_MENU_ITEM_COUNT;i++)if(menuTextMesh[i * 2 + 1])menuTextMesh[i * 2 + 1]->setColor(0x000000FF);
    SSwhiteQuad = new TwoDColorQuad(0xFFFFFFFF);
    SSwhiteQuad->posZ = 0.08f;
    string slotTextString[] = {"Slot0", "Slot1", "Slot2", "Slot3"};
    for(unsigned int i = 0;i < STATE_SLOT_COUNT;i++){
        if(ttfFont){
            slotText[i * 2] = new TextMesh(ttfFont, ttfFontSize, 25, slotTextString[i], 0xFFFFFFFF);
            setTextMeshScale(slotText[i * 2]);
            slotText[i * 2]->posZ = 0.07f;
            setTextMeshPos(slotText[i * 2], 894, 93 + 140 * i + ((114 - slotText[i * 2]->realTexHeight) / 2));
            slotText[i * 2 + 1] = new TextMesh(ttfFont, ttfFontSize, 25, slotTextString[i], 0xFF0000FF);
            setTextMeshScale(slotText[i * 2 + 1]);
            slotText[i * 2 + 1]->posZ = 0.07f;
            setTextMeshPos(slotText[i * 2 + 1], 894, 93 + 140 * i + ((114 - slotText[i * 2 + 1]->realTexHeight) / 2));
        }else{
            slotText[i * 2] = nullptr;
            slotText[i * 2 + 1] = nullptr;
        }
        slotDisplayRect[i * 2] = new SimpleRect(714, 93 + 140 * i, 145, 114, 0.08f);
        slotDisplayRect[i * 2 + 1] = new SimpleRect(714, 93 + 140 * i, 417, 114, 0.08f);
    }
}

void StateSave::prepareForActive(void){
    string texPath;
    for(unsigned int i = 0;i < STATE_SLOT_COUNT;i++){
        if(stateSnap[i])delete stateSnap[i];
        if(pApp->alreadyLoadROM){
            if(pApp->isStateSaveExsist(i)){
                string stateSnapPath = pApp->romPathBase + "_slot" + intToString(i) + ".bmp";
                FILE *f = getFileSizeAndPointer(stateSnapPath.c_str(), nullptr);
                if(f){
                    fclose(f);
                    texPath = stateSnapPath;
                }else{
                    texPath = "romfs:/texture/state_save.png";
                }
            }else{
                texPath = "romfs:/texture/no_data.png";
            }
        }else{
            texPath = "romfs:/texture/no_data.png";
        }
        stateSnap[i] = new TwoDTexQuad(texPath);
        stateSnap[i]->posZ = 0.085f;
        setTwoDColorQuadPosAndScale(stateSnap[i], 714, 93 + 140 * i, 145, 114);
    }
}

void StateSave::render(){
    if(!renderFlag)return;
    unsigned int curItemindexBackup = curItemindex;
    curItemindex = 1;
    if(isSave)curItemindex = 2;
    MainMenuRender();
    curItemindex = curItemindexBackup;
    SSwhiteQuad->render();
    for(unsigned int i = 0;i < STATE_SLOT_COUNT;i++){
        if(stateSnap[i])stateSnap[i]->render();
        slotDisplayRect[i * 2]->render();
        slotDisplayRect[i * 2 + 1]->render();
        if(curItemindex == i){
            if(ttfFont)slotText[i * 2 + 1]->render();
        }else{
            if(ttfFont)slotText[i * 2]->render();
        }
    }
}

void StateSave::run1Fr(unsigned int joyD, unsigned int jpyU, unsigned int joyH){
    renderFlag = false;
    if(!active)return;
    renderFlag = true;
    if((joyD & (1 << JOY_DOWN)) != 0){//↓ ボタン押された
        if(curItemindex < (itemCount - 1))curItemindex++;
    }
    if((joyD & (1 << JOY_UP)) != 0){//↑ ボタン押された
        if(curItemindex > 0)curItemindex--;
    }
    if((joyD & (1 << JOY_B)) != 0){//B ボタン押された
        pApp->menu[1]->active = true;
        active = false;
        pApp->skipMenuFlag = true;
    }
    if((joyD & (1 << JOY_A)) != 0){
        if(!pApp->alreadyLoadROM)return;
        //ステートセーブ or ロード
        if(isSave){
            pApp->saveState(curItemindex);
            prepareForActive();//テクスチャ更新
        }else{
            if(pApp->isStateSaveExsist(curItemindex)){
                if(pApp->loadState(curItemindex)){
                    pApp->menu[0]->active = true;
                    active = false;
                    pApp->skipMenuFlag = true;
                }
            }
        }
    }
    unsigned int mainMenuSelectndex = 1;
    if(isSave)mainMenuSelectndex = 2;
    notAlreadyLoadROMBg->run1Fr(0.0f);
    bgLMesh->run1Fr(0.0f);
    bgRMesh->run1Fr(0.0f);
    s9xRenderer->run1Fr(0.0f);
    bokashiDarkQuad->run1Fr(0.0f);
    whiteQuad->posY = ((((mainMenuSelectndex * 48 + 165) / (float)SCREEN_H) + (48 / (float)SCREEN_H) / 2) * -2.0f) + 1.0f;
    whiteQuad->run1Fr(0.0f);
    VerticalLine->run1Fr(0.0f);
    for(unsigned int i = 0;i < MAIN_MENU_ITEM_COUNT * 2;i++)if(menuTextMesh[i])menuTextMesh[i]->run1Fr(0.0f);
    for(unsigned int i = 0;i < STATE_SLOT_COUNT;i++){
        if(stateSnap[i])stateSnap[i]->run1Fr(0.0f);
        if(ttfFont){
            slotText[i * 2]->run1Fr(0.0f);
            slotText[i * 2 + 1]->run1Fr(0.0f);
        }
        slotDisplayRect[i * 2]->run1Fr(0.0f);
        slotDisplayRect[i * 2 + 1]->run1Fr(0.0f);
    }
    setTwoDColorQuadPosAndScale(SSwhiteQuad, 714 + 145, 93 + 140 * curItemindex, 272, 114);
    SSwhiteQuad->run1Fr(0.0f);
    return;
}

StateSave::~StateSave(void){
    for(unsigned int i = 0;i < STATE_SLOT_COUNT;i++){
        if(stateSnap[i])delete stateSnap[i];
        if(ttfFont){
            delete slotText[i * 2];
            delete slotText[i * 2 + 1];
        }
        delete slotDisplayRect[i * 2];
        delete slotDisplayRect[i * 2 + 1];
    }
    delete SSwhiteQuad;
}

bool ROMLoadMenu::isDir(string fullPath){
    struct stat st;
    int result = stat(fullPath.c_str(), &st);
    if(result != 0)return false;
    if((st.st_mode & S_IFMT) == S_IFDIR)return true;
    return false;
}

string ROMLoadMenu::getExtension(string path){
    const char *c_str = path.c_str();
    int index = strlen(c_str) - 1;
    while(index > -1){
        if(c_str[index] == '/')return "";
        if(c_str[index] == '.'){
            return c_str + index;
        }
        index--;
    }
    return "";
}

bool ROMLoadMenu::displayThisPath(string fullPath){
    if(isDir(fullPath))return true;
    string ext = getExtension(fullPath);
    if(getUpperText(ext) == ".SMC" || getUpperText(ext) == ".SFC")return true;
    return false;
}

void ROMLoadMenu::fetchPaths(void){
    displayEntries.clear();
    displayEntries.shrink_to_fit();
    curItemindex = 0;
    itemCount = 0;
    for(unsigned int i = 0;i < fm->entries.size();i++){
        if(!displayThisPath(fm->getFullPath(i)))continue;
        displayEntries.push_back(fm->entries[i]);
        itemCount++;
    }
    itemCount++;//Backの分
}

void ROMLoadMenu::updateTextures(void){
    page = curItemindex / PATH_DISPLAY_COUNT;
    if(!ttfFont)return;
    if(curDirDisplay)delete curDirDisplay;
    curDirDisplay = new TextMesh(ttfFont, ttfFontSize, 25, fm->curPath.c_str(), 0xFFFFFFFF);
    setTextMeshScale(curDirDisplay);
    setTextMeshPos(curDirDisplay, 714, 93 + ((48 - (int)curDirDisplay->realTexHeight) / 2));
    string renderTargetText;
    for(int i = 0;i < PATH_DISPLAY_COUNT * 2;i++)if(pathDisplay[i])delete pathDisplay[i];
    for(int i = 0;i < PATH_DISPLAY_COUNT;i++){
        int itemIndex = PATH_DISPLAY_COUNT * page + i;
        if(itemIndex == 0){
            renderTargetText = "../";
        }else if(itemIndex < itemCount){
            if(displayEntries[itemIndex - 1].isDir){
                renderTargetText = displayEntries[itemIndex - 1].name + "/";
            }else{
                renderTargetText = displayEntries[itemIndex - 1].name;
            }
        }else{
            renderTargetText = "";
        }
        pathDisplay[i * 2] = new TextMesh(ttfFont, ttfFontSize, 25, renderTargetText.c_str(), 0xFFFFFFFF);
        setTextMeshScale(pathDisplay[i * 2]);
        setTextMeshPos(pathDisplay[i * 2], 714, 93 + 60 + 12 + 48 * i + ((48 - (int)pathDisplay[i * 2]->realTexHeight) / 2));
        pathDisplay[i * 2]->posZ = 0.07f;
        pathDisplay[i * 2 + 1] = new TextMesh(ttfFont, ttfFontSize, 25, renderTargetText.c_str(), 0xFF0000FF);
        setTextMeshScale(pathDisplay[i * 2 + 1]);
        setTextMeshPos(pathDisplay[i * 2 + 1], 714, 93 + 60 + 12 + 48 * i + ((48 - (int)pathDisplay[i * 2 + 1]->realTexHeight) / 2));
        pathDisplay[i * 2 + 1]->posZ = 0.07f;
    }
}

ROMLoadMenu::ROMLoadMenu(App *a, unsigned char *aTtfFont, unsigned int aTtfFontSize) : MainMenu(a, aTtfFont, aTtfFontSize){
    for(unsigned int i = 0;i < MAIN_MENU_ITEM_COUNT;i++)if(menuTextMesh[i * 2 + 1])menuTextMesh[i * 2 + 1]->setColor(0x000000FF);
    for(unsigned int i = 0;i < PATH_DISPLAY_COUNT * 2;i++)pathDisplay[i] = nullptr;
    fm = new FileManager("sdmc:/");
    RLwhiteQuad = new TwoDColorQuad(0xFFFFFFFF);
    setTwoDColorQuadScale(RLwhiteQuad, 1240 - 740, 48);
    RLwhiteQuad->posX = ((714 / (float)SCREEN_W) + ((1240 - 740) / (float)SCREEN_W) / 2) * 2.0f - 1.0f;
    horizontalLine = new TwoDColorQuad(0xFFFFFFFF);
    setTwoDColorQuadScale(horizontalLine, 588, 1);
    horizontalLine->posX = ((652 / (float)SCREEN_W) + (588 / (float)SCREEN_W) / 2) * 2.0f - 1.0f;
    horizontalLine->posY = (((153 / (float)SCREEN_H) + (1 / (float)SCREEN_H) / 2) * -2.0f) + 1.0f;
}

void ROMLoadMenu::run1Fr(unsigned int joyD, unsigned int jpyU, unsigned int joyH){
    renderFlag = false;
    if(!active)return;
    renderFlag = true;
    int pagePrev = page;
    if((joyD & (1 << JOY_B)) != 0){//B ボタン押された
        active = false;
        pApp->menu[1]->active = true;
        pApp->skipMenuFlag = true;
    }
    if((joyD & (1 << JOY_A)) != 0){//A ボタン押された
        if(curItemindex == 0){//一つ上のディレクトリへ
            fm->Back();
            fetchPaths();
            updateTextures();
        }else{
            string selectedPath = fm->curPath + "/" + displayEntries[curItemindex - 1].name;
            if(fm->curPath.at(fm->curPath.size() - 1) == '/')selectedPath = fm->curPath + displayEntries[curItemindex - 1].name;
            if(displayEntries[curItemindex - 1].isDir){
                fm->OpenPath(selectedPath);
                fetchPaths();
                updateTextures();
            }else{
                pApp->loadRom(selectedPath);
                active = false;
                pApp->menu[0]->active = true;
                pApp->skipMenuFlag = true;
            }
        }
    }
    if((joyD & (1 << JOY_DOWN)) != 0){//↓ ボタン押された
        if(curItemindex < (itemCount - 1))curItemindex++;
        page = curItemindex / 10;
        if(page != pagePrev)updateTextures();
    }
    if((joyD & (1 << JOY_UP)) != 0){//↑ ボタン押された
        if(curItemindex > 0)curItemindex--;
        page = curItemindex / 10;
        if(page != pagePrev)updateTextures();
    }
    if((joyD & (1 << JOY_LEFT)) != 0){//← ボタン押された
        if(curItemindex > 9){
            curItemindex -= 10;
        }else{
            curItemindex = 0;
        }
        page = curItemindex / 10;
        if(page != pagePrev)updateTextures();
    }
    if((joyD & (1 << JOY_RIGHT)) != 0){//→ ボタン押された
        if((curItemindex + 10) < itemCount){
            curItemindex += 10;
        }else{
            curItemindex = itemCount - 1;
        }
        page = curItemindex / 10;
        if(page != pagePrev)updateTextures();
    }
    notAlreadyLoadROMBg->run1Fr(0.0f);
    bgLMesh->run1Fr(0.0f);
    bgRMesh->run1Fr(0.0f);
    s9xRenderer->run1Fr(0.0f);
    bokashiDarkQuad->run1Fr(0.0f);
    whiteQuad->posY = ((((4 * 48 + 165) / (float)SCREEN_H) + (48 / (float)SCREEN_H) / 2) * -2.0f) + 1.0f;
    whiteQuad->run1Fr(0.0f);
    VerticalLine->run1Fr(0.0f);
    for(unsigned int i = 0;i < MAIN_MENU_ITEM_COUNT * 2;i++)if(menuTextMesh[i])menuTextMesh[i]->run1Fr(0.0f);
    if(curDirDisplay)curDirDisplay->run1Fr(0.0f);
    for(unsigned int i = 0;i < PATH_DISPLAY_COUNT * 2;i++)if(pathDisplay[i])pathDisplay[i]->run1Fr(0.0f);
    RLwhiteQuad->posY = (((((curItemindex % PATH_DISPLAY_COUNT) * 48 + 93 + 60 + 12) / (float)SCREEN_H) + (48 / (float)SCREEN_H) / 2) * -2.0f) + 1.0f;
    RLwhiteQuad->posZ = 0.08f;
    RLwhiteQuad->run1Fr(0.0f);
    horizontalLine->run1Fr(0.0f);
}

void ROMLoadMenu::render(void){
    if(!renderFlag)return;
    unsigned int curItemindexBackup = curItemindex;
    curItemindex = 4;
    MainMenuRender();
    curItemindex = curItemindexBackup;
    if(curDirDisplay)curDirDisplay->render();
    horizontalLine->render();
    RLwhiteQuad->render();
    page = curItemindex / PATH_DISPLAY_COUNT;
    unsigned int displayCurItemindex = curItemindex % PATH_DISPLAY_COUNT;
    for(unsigned int i = 0;i < PATH_DISPLAY_COUNT;i++){
        if(i == displayCurItemindex){
            if(pathDisplay[i * 2 + 1])pathDisplay[i * 2 + 1]->render();
        }else{
            if(pathDisplay[i * 2])pathDisplay[i * 2]->render();
        }
    }
}

ROMLoadMenu::~ROMLoadMenu(void){
    delete fm;
    delete RLwhiteQuad;
    delete horizontalLine;
    if(curDirDisplay)delete curDirDisplay;
    for(unsigned int i = 0;i < PATH_DISPLAY_COUNT * 2;i++)if(pathDisplay[i])delete pathDisplay[i];
}