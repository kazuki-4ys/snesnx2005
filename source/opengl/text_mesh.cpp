#include "text_mesh.h"

#define BMP_HEAD_SIZE 0x36

const unsigned char bmpHead[] = {
0x42, 0x4D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x28, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x20, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

bool writeRawTexToBmp(unsigned char *data, unsigned int width, unsigned int height, const char *destPath){
    unsigned int destSize = width * height * 4 + BMP_HEAD_SIZE;
    unsigned char *dest = (unsigned char*)calloc(sizeof(unsigned char), destSize);
    memcpy(dest, bmpHead, BMP_HEAD_SIZE);
    uint32ToBytes(dest + 2, destSize, true);
    uint32ToBytes(dest + 0x12, width, true);
    uint32ToBytes(dest + 0x16, height, true);
    for(unsigned int y = 0;y < height;y++){
        for(unsigned int x = 0;x < width;x++){
            unsigned char *srcPixel = data + (width * y + x) * 4;
            unsigned char *destPixel = dest + BMP_HEAD_SIZE + (width * ((height - 1) - y) + x) * 4;
            unsigned char r = *(srcPixel + 0);
            unsigned char g = *(srcPixel + 1);
            unsigned char b = *(srcPixel + 2);
            unsigned char a = *(srcPixel + 3);
            *(destPixel + 0) = b;
            *(destPixel + 1) = g;
            *(destPixel + 2) = r;
            *(destPixel + 3) = a;
        }
    }
    bool result = bytesToFile(dest, destSize, destPath);
    free(dest);
    return result;
}

//from https://github.com/benikabocha/unicode_conv/blob/master/UnicodeConv.cpp

bool IsU8LaterByte(char ch) {
    return 0x80 <= uint8_t(ch) && uint8_t(ch) < 0xC0;
}

unsigned int GetU8ByteCount(unsigned char ch) {
    if (0 <= uint8_t(ch) && uint8_t(ch) < 0x80) {
        return 1;
    }
    if (0xC2 <= uint8_t(ch) && uint8_t(ch) < 0xE0) {
        return 2;
    }
    if (0xE0 <= uint8_t(ch) && uint8_t(ch) < 0xF0) {
        return 3;
    }
    if (0xF0 <= uint8_t(ch) && uint8_t(ch) < 0xF8) {
        return 4;
    }
    return 0;
}

unsigned int ConvChU8ToU32(const unsigned char *u8Ch, unsigned int *u32Ch) {
    int numBytes = GetU8ByteCount(u8Ch[0]);
    if (numBytes == 0) {
        return 0;
    }
    switch (numBytes) {
        case 1:
            *u32Ch = char32_t(uint8_t(u8Ch[0]));
            return 1;
        case 2:
            if (!IsU8LaterByte(u8Ch[1])) {
                return 0;
            }
            if ((uint8_t(u8Ch[0]) & 0x1E) == 0) {
                return 0;
            }

            *u32Ch = char32_t(u8Ch[0] & 0x1F) << 6;
            *u32Ch |= char32_t(u8Ch[1] & 0x3F);
            return 2;
        case 3:
            if (!IsU8LaterByte(u8Ch[1]) || !IsU8LaterByte(u8Ch[2])) {
                return 0;
            }
            if ((uint8_t(u8Ch[0]) & 0x0F) == 0 &&
                (uint8_t(u8Ch[1]) & 0x20) == 0) {
                return 0;
            }

            *u32Ch = char32_t(u8Ch[0] & 0x0F) << 12;
            *u32Ch |= char32_t(u8Ch[1] & 0x3F) << 6;
            *u32Ch |= char32_t(u8Ch[2] & 0x3F);
            return 3;
        case 4:
            if (!IsU8LaterByte(u8Ch[1]) || !IsU8LaterByte(u8Ch[2]) ||
                !IsU8LaterByte(u8Ch[3])) {
                return 0;
            }
            if ((uint8_t(u8Ch[0]) & 0x07) == 0 &&
                (uint8_t(u8Ch[1]) & 0x30) == 0) {
                return 0;
            }

            *u32Ch = char32_t(u8Ch[0] & 0x07) << 18;
            *u32Ch |= char32_t(u8Ch[1] & 0x3F) << 12;
            *u32Ch |= char32_t(u8Ch[2] & 0x3F) << 6;
            *u32Ch |= char32_t(u8Ch[3] & 0x3F);
            return 4;
        default:
            return 0;
    }

    return true;
}

unsigned int *stringToU32Str(string _src){
    const char *src = _src.c_str();
    unsigned int srcLen = strlen(src);
    unsigned int *dest = (unsigned int*)calloc(sizeof(unsigned int), srcLen + 1);
    unsigned int curDestIndex = 0;
    unsigned int curSrcIndex = 0;
    while(true){
        if(src[curSrcIndex] == '\0')break;
        curSrcIndex += ConvChU8ToU32((const unsigned char*)(src + curSrcIndex), dest + curDestIndex);
        curDestIndex++;
    }
    return dest;
}

void getStringWidth(FT_Face face, unsigned int *u32Text, unsigned int *size){
    unsigned int dest = 0;
    unsigned int maxHeight = 0;
    unsigned int curIndex = 0;
    while(u32Text[curIndex] != '\0'){
        FT_Bitmap bitmap;
        FT_Load_Char(face, u32Text[curIndex], FT_LOAD_RENDER);
        bitmap = face->glyph->bitmap;
        dest += bitmap.width;
        if(maxHeight < bitmap.rows)maxHeight = bitmap.rows;
        curIndex++;
    }
    *size = dest;
    *(size + 1) = maxHeight;
}

TextMesh::TextMesh(unsigned char *ttf, unsigned int ttfSize, unsigned int aHeight, string aText, unsigned int aColor) : TwoDTexQuad(""){
    height = aHeight;
    color = aColor;
    text = aText;
    FT_New_Memory_Face(ftLibrary, ttf, ttfSize, 0, &face);
    //FT_Set_Char_Size(face, 0, 64 * height, 300, 300);
    FT_Set_Pixel_Sizes(face, 0, height);
    setText(text, aColor);
}

void TextMesh::setText(string aText){
    text = aText;
    unsigned int *u32Text = stringToU32Str(text);
    unsigned int curIndex = 0;
    unsigned int size[2];
    getStringWidth(face, u32Text, size);
    realTexWidth = size[0];
    unsigned int realTexHeight2 = size[1];
    realTexHeight = size[1] * 1.5f;
    unsigned char *rawTex = (unsigned char*)calloc(sizeof(unsigned char), realTexWidth * realTexHeight * 4);
    //memset(rawTex, 0, realTexWidth * realTexHeight * 4);
    unsigned int curCharX = 0;
    unsigned char *curTargetPixel;
    FT_Bitmap bitmap;
    while(u32Text[curIndex] != '\0'){
        FT_Load_Char(face, u32Text[curIndex], FT_LOAD_RENDER);
        bitmap = face->glyph->bitmap;
        for(unsigned int y = 0;y < bitmap.rows;y++){
            //if((y - face->glyph->bitmap_top) < 0)continue;
            if((y + (realTexHeight2 - face->glyph->bitmap_top)) >= realTexHeight)break;
            for(unsigned int x = 0;x < bitmap.width;x++){
                if(curCharX + x >= realTexWidth)break;
                //curTargetPixel = rawTex + 4 * (((realTexHeight - 1) - y) * realTexWidth + curCharX + x);
                curTargetPixel = rawTex + 4 * (((realTexHeight - 1) - (y + (realTexHeight2 - face->glyph->bitmap_top))) * realTexWidth + curCharX + x);
                unsigned char srcPixel = *((unsigned char*)bitmap.buffer + bitmap.width * y + x);
                *(curTargetPixel + 0) = (color >> 24) & 0xFF;
                *(curTargetPixel + 1) = (color >> 16) & 0xFF;
                *(curTargetPixel + 2) = (color >> 8) & 0xFF;
                *(curTargetPixel + 3) = (unsigned char)(((color & 0xFF) * (srcPixel / 255.0f)) + 0.4999f);
            }
        }
        curCharX += bitmap.width;
        curIndex++;
    }
    //writeRawTexToBmp(rawTex, realTexWidth, realTexHeight, "sdmc:/test.bmp");
    glBindTexture(GL_TEXTURE_2D, s_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, realTexWidth, realTexHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, rawTex);
    free(rawTex);
    free(u32Text);
}

void TextMesh::setText(string aText, unsigned int aColor){
    text = aText;
    color = aColor;
    setText(text);
}

void TextMesh::setColor(unsigned int aColor){
    color = aColor;
    setText(text);
}

TextMesh::~TextMesh(void){
    FT_Done_Face(face);
}