#include "mesh.h"

constexpr auto TAU = glm::two_pi<float>();

BaseMesh::BaseMesh(void){
    //Matrix 初期化
    projMtx = glm::perspective(40.0f*TAU/360.0f, 1280.0f/720.0f, 0.01f, 1000.0f);
    //viewMtx = glm::translate(viewMtx, glm::vec3{0.0f, 0.0f, -3.0f});
    viewMtx = glm::translate(viewMtx, glm::vec3{0.0f, 0.0f, -5.0f});

    shader = new BaseShader();
    initVaoVbo();
}

void BaseMesh::initVaoVbo(){
    //vao, vbo作成
    glGenVertexArrays(1, &s_vao);
    glGenBuffers(1, &s_vbo);
    
    glBindBuffer(GL_ARRAY_BUFFER, s_vbo);
    setVertexBuffer();

    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(s_vao);

    shader->setInputLayout();

    //???
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void BaseMesh::setVertexBuffer(void){
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_list), vertex_list, GL_STATIC_DRAW);//頂点データをセット
}

void BaseMesh::run1Fr(float timeDelta){
    mdlMtx = glm::mat4{1.0};
    mdlMtx = glm::rotate(mdlMtx, timeDelta * TAU * 0.234375f, glm::vec3{1.0f, 0.0f, 0.0f});
    mdlMtx = glm::rotate(mdlMtx, timeDelta * TAU * 0.234375f / 2.0f, glm::vec3{0.0f, 1.0f, 0.0f});
    return;
}

void BaseMesh::render(void){
    glBindVertexArray(s_vao);

    shader->useThis();
    //shader->setInputLayout();

    //uniform変数をシェーダーに渡す
    glUniformMatrix4fv(shader->loc_mdlMtx, 1, GL_FALSE, glm::value_ptr(mdlMtx));
    glUniformMatrix4fv(shader->loc_viewMtx, 1, GL_FALSE, glm::value_ptr(viewMtx));
    glUniformMatrix4fv(shader->loc_projMtx, 1, GL_FALSE, glm::value_ptr(projMtx));

    //glBindVertexArray(s_vao); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
    glDrawArrays(GL_TRIANGLES, 0, vertex_list_count);
}

BaseMesh::~BaseMesh(void){
    glDeleteBuffers(1, &s_vbo);
    glDeleteVertexArrays(1, &s_vao);
    delete shader;
}

TexMesh::TexMesh(string vshPath, string fshPath, string texPath) : BaseMesh(){
    glDeleteBuffers(1, &s_vbo);
    glDeleteVertexArrays(1, &s_vao);

    //Matrix 初期化
    projMtx = glm::perspective(40.0f*TAU/360.0f, 1280.0f/720.0f, 0.01f, 1000.0f);
    //viewMtx = glm::translate(viewMtx, glm::vec3{0.0f, 0.0f, -3.0f});
    //viewMtx = glm::translate(viewMtx, glm::vec3{0.0f, 0.0f, -5.0f});

    shader = (BaseShader*)(new BaseTexShader(vshPath, fshPath));
    initVaoVbo();

    glGenTextures(1, &s_tex);
    glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture
    glBindTexture(GL_TEXTURE_2D, s_tex);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if(texPath == "")return;//テクスチャの生成スキップ

    int width, height, nchan;
    unsigned int texRawSize;
    unsigned char *texRaw = fileToBytes(texPath.c_str(), &texRawSize);
    stbi_set_flip_vertically_on_load(true);
    stbi_uc* img = stbi_load_from_memory((const stbi_uc*)texRaw, texRawSize, &width, &height, &nchan, 4);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
    stbi_image_free(img);
    free(texRaw);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void TexMesh::setVertexBuffer(void){
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_list), vertex_list, GL_STATIC_DRAW);//頂点データをセット
}

void TexMesh::render(void){
    glBindVertexArray(s_vao);
    

    shader->useThis();
    //shader->setInputLayout();

    //uniform変数をシェーダーに渡す
    glUniformMatrix4fv(shader->loc_mdlMtx, 1, GL_FALSE, glm::value_ptr(mdlMtx));
    glUniformMatrix4fv(shader->loc_viewMtx, 1, GL_FALSE, glm::value_ptr(viewMtx));
    glUniformMatrix4fv(shader->loc_projMtx, 1, GL_FALSE, glm::value_ptr(projMtx));
    glUniform1i(((BaseTexShader*)shader)->loc_tex, 0); // texunit 0

    glBindTexture(GL_TEXTURE_2D, s_tex);
    //glBindVertexArray(s_vao); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
    glDrawArrays(GL_TRIANGLES, 0, vertex_list_count);
}

TexMesh::~TexMesh(void){
    glDeleteTextures(1, &s_tex);
}

TwoDTexQuad::TwoDTexQuad(string texPath) : TexMesh("romfs:/shader/2d_base_tex.vert", "romfs:/shader/base_tex.frag", texPath){
    glDeleteBuffers(1, &s_vbo);
    glDeleteVertexArrays(1, &s_vao);
    initVaoVbo();
    return;
}

void TwoDTexQuad::setVertexBuffer(void){
    Vertex TwodQuad[6] = {
    //左上の三角形
    { {-1.0f, 1.0f, 0.5f}, {0.0f, 1.0f}, {0.0f, 0.0f, +1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
    { {-1.0f, -1.0f, 0.5f}, {0.0f, 0.0f}, {0.0f, 0.0f, +1.0f}, {1.0f, 1.0f, 1.0f, 1.0f} },
    { {1.0f, 1.0f, 0.5f}, {1.0f, 1.0f}, {0.0f, 0.0f, +1.0f}, {1.0f, 1.0f, 1.0f, 1.0f} },

    //右下の三角形
    { {1.0f, 1.0f, 0.5f}, {1.0f, 1.0f}, {0.0f, 0.0f, +1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
    { {-1.0f, -1.0f, 0.5f}, {0.0f, 0.0f}, {0.0f, 0.0f, +1.0f}, {1.0f, 1.0f, 1.0f, 1.0f} },
    { {1.0f, -1.0f, 0.5f}, {1.0f, 0.0f}, {0.0f, 0.0f, +1.0f}, {1.0f, 1.0f, 1.0f, 1.0f} },
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(TwodQuad), TwodQuad, GL_STATIC_DRAW);
}

void TwoDTexQuad::run1Fr(float timeDelta){
    float realZ = posZ * 0.95f + 0.025f;
    mdlMtx = glm::translate(glm::mat4{1.0}, glm::vec3{posX, posY, realZ});
    mdlMtx = glm::scale(mdlMtx, glm::vec3{scaleX, scaleY, 1.0f});
}

void TwoDTexQuad::render(void){
    glBindVertexArray(s_vao);

    shader->useThis();
    //shader->setInputLayout();

    //uniform変数をシェーダーに渡す
    glUniformMatrix4fv(shader->loc_mdlMtx, 1, GL_FALSE, glm::value_ptr(mdlMtx));
    glUniformMatrix4fv(shader->loc_viewMtx, 1, GL_FALSE, glm::value_ptr(viewMtx));
    glUniformMatrix4fv(shader->loc_projMtx, 1, GL_FALSE, glm::value_ptr(projMtx));
    glUniform1i(((BaseTexShader*)shader)->loc_tex, 0); // texunit 0

    glBindTexture(GL_TEXTURE_2D, s_tex);
    //glBindVertexArray(s_vao); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

TwoDDynamicTexQuad::TwoDDynamicTexQuad(void) : TwoDTexQuad(""){
    return;
}

void TwoDDynamicTexQuad::setTex(unsigned char *data, unsigned int width, unsigned int height){
    isAlreadySetTex = true;
    glBindTexture(GL_TEXTURE_2D, s_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

void TwoDDynamicTexQuad::render(void){
    if(!isAlreadySetTex)return;
    glBindVertexArray(s_vao);

    shader->useThis();
    //shader->setInputLayout();

    //uniform変数をシェーダーに渡す
    glUniformMatrix4fv(shader->loc_mdlMtx, 1, GL_FALSE, glm::value_ptr(mdlMtx));
    glUniformMatrix4fv(shader->loc_viewMtx, 1, GL_FALSE, glm::value_ptr(viewMtx));
    glUniformMatrix4fv(shader->loc_projMtx, 1, GL_FALSE, glm::value_ptr(projMtx));
    glUniform1i(((BaseTexShader*)shader)->loc_tex, 0); // texunit 0

    glBindTexture(GL_TEXTURE_2D, s_tex);
    //glBindVertexArray(s_vao); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

TwoDColorQuad::TwoDColorQuad(unsigned color) : TwoDDynamicTexQuad(){
    unsigned char tex[4];
    tex[0] = (unsigned char)((color >> 24) & 0xFF);
    tex[1] = (unsigned char)((color >> 16) & 0xFF);
    tex[2] = (unsigned char)((color >> 8) & 0xFF);
    tex[3] = (unsigned char)((color >> 0) & 0xFF);
    setTex(tex, 1, 1);
    return;
}