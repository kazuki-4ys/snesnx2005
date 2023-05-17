#include "shader.h"

GLuint createAndCompileShader(GLenum type, const char* source){
    GLint success;
    GLchar msg[512];

    GLuint handle = glCreateShader(type);
    if (!handle)
    {
        TRACE("%u: cannot create shader", type);
        return 0;
    }
    glShaderSource(handle, 1, &source, nullptr);
    glCompileShader(handle);
    glGetShaderiv(handle, GL_COMPILE_STATUS, &success);

    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(handle, sizeof(msg), nullptr, msg);
        TRACE("%u: %s\n", type, msg);
        glDeleteShader(handle);
        return 0;
    }

    return handle;
}

BaseShader::BaseShader(string vshPath, string fshPath){
    vShaderPath = vshPath;
    fShaderPath = fshPath;
    compileShader();
    getUinformLocations();
}

BaseShader::BaseShader(void){
    vShaderPath = "romfs:/shader/base.vert";
    fShaderPath = "romfs:/shader/base.frag";
    compileShader();
    getUinformLocations();
}

void BaseShader::getUinformLocations(void){
    loc_mdlMtx = glGetUniformLocation(s_program, "mdlMtx");
    loc_viewMtx = glGetUniformLocation(s_program, "viewMtx");
    loc_projMtx = glGetUniformLocation(s_program, "projMtx");
}

void BaseShader::setInputLayout(void){
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texcoord));
    glEnableVertexAttribArray(3);

}

void BaseShader::compileShader(void){
    unsigned int fileSize;
    s_program = glCreateProgram();
    unsigned char *vshText = fileToBytes(vShaderPath.c_str(), &fileSize);
    unsigned char *fshText = fileToBytes(fShaderPath.c_str(), &fileSize);
    GLint vsh = createAndCompileShader(GL_VERTEX_SHADER, (const char*)vshText);
    if(vsh == 0)exit(0);
    GLint fsh = createAndCompileShader(GL_FRAGMENT_SHADER, (const char*)fshText);
    glAttachShader(s_program, vsh);
    glAttachShader(s_program, fsh);
    glLinkProgram(s_program);
    GLint success;
    glGetProgramiv(s_program, GL_LINK_STATUS, &success);
    if (success == GL_FALSE)
    {
        char buf[512];
        glGetProgramInfoLog(s_program, sizeof(buf), nullptr, buf);
        TRACE("Link error: %s", buf);
    }
    glDeleteShader(vsh);
    glDeleteShader(fsh);
    free(vshText);
    free(fshText);
}

void BaseShader::useThis(void){
    glDisable(GL_CULL_FACE);//裏面も描画
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);//透過設定
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);//透過設定
    glDepthFunc(GL_LESS);
    glUseProgram(s_program);
}

GLuint BaseShader::getUniLocByName(string uniName){
    return glGetUniformLocation(s_program, uniName.c_str());
}

BaseShader::~BaseShader(void){
    glDeleteProgram(s_program);
}

BaseTexShader::BaseTexShader(string vshPath, string fshPath) : BaseShader(vshPath, fshPath){
    glDeleteProgram(s_program);
    compileShader();
    getUinformLocations();
}

BaseTexShader::BaseTexShader(void) : BaseShader(){
    glDeleteProgram(s_program);
    vShaderPath = "romfs:/shader/base_tex.vert";
    fShaderPath = "romfs:/shader/base_tex.frag";
    compileShader();
    getUinformLocations();
}

void BaseTexShader::getUinformLocations(void){
    loc_mdlMtx = glGetUniformLocation(s_program, "mdlMtx");
    loc_viewMtx = glGetUniformLocation(s_program, "viewMtx");
    loc_projMtx = glGetUniformLocation(s_program, "projMtx");
    loc_tex = glGetUniformLocation(s_program, "projMtx");
}