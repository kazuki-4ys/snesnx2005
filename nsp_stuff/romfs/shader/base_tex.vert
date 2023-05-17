#version 330 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec4 vColor;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec2 inTexCoord;

out vec4 ourColor;
out vec2 vtxTexCoord;

uniform mat4 mdlMtx;
uniform mat4 viewMtx;
uniform mat4 projMtx;

void main(){
    mat4 MVPmatrix = projMtx * viewMtx * mdlMtx;
    gl_Position = MVPmatrix * vec4(inPos, 1.0);
    vtxTexCoord = inTexCoord;
    ourColor = vColor;
}