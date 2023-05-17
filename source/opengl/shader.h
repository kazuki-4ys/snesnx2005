#ifndef _BASE_SHADER_H_
#define _BASE_SHADER_H_

#include <string>

#include <EGL/egl.h>    // EGL library
#include <EGL/eglext.h> // EGL extensions
#include <glad/glad.h>  // glad library (OpenGL loader)

#define GLM_FORCE_PURE
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../utils.h"

#ifndef ENABLE_NXLINK
#define TRACE(fmt,...) ((void)0)
#else
#include <unistd.h>
#define TRACE(fmt,...) printf("%s: " fmt "\n", __PRETTY_FUNCTION__, ## __VA_ARGS__)
#endif

using namespace std;

typedef struct{
    float position[3];
    float texcoord[2];
    float normal[3];
    float color[4];
}Vertex;

class BaseShader{
    protected:
        string vShaderPath = "";
        string fShaderPath = "";
        virtual void getUinformLocations(void);
        virtual void compileShader(void);
    public:
        GLuint s_program;

        GLint loc_mdlMtx;
        GLint loc_viewMtx;
        GLint loc_projMtx;

        BaseShader(string vshPath, string fshPath);
        BaseShader(void);

        virtual void useThis(void);
        virtual void setInputLayout(void);
        GLuint getUniLocByName(string uniName);
        
        virtual ~BaseShader(void);
};

class BaseTexShader : public BaseShader{
    protected:
        void getUinformLocations(void);
    public:
        GLint loc_tex;
        BaseTexShader(string vshPath, string fshPath);
        BaseTexShader(void);
};

#endif//_BASE_SHADER_H_