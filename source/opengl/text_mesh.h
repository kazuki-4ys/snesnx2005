#ifndef _TEXT_MESH_H_
#define _TEXT_MESH_H_

#include <malloc.h>
#include <string.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <string>

#include "mesh.h"

using namespace std;

extern FT_Library ftLibrary;

class TextMesh : public TwoDTexQuad{
        FT_Face face;
        unsigned int height;
        unsigned int color;
        string text;
    public:
        unsigned int realTexHeight;
        unsigned int realTexWidth;
        TextMesh(unsigned char *ttf, unsigned int ttfSize, unsigned int aHeight, string text, unsigned int aColor);
        void setText(string text);
        void setText(string text, unsigned int aColor);
        void setColor(unsigned int aColor);
        ~TextMesh(void);
};

#endif//_TEXT_MESH_H_