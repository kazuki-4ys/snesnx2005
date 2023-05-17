#version 330 core

in vec4 ourColor;
in vec2 vtxTexCoord;

uniform sampler2D tex;

out vec4 fragColor;

void main(){
    fragColor = texture(tex, vtxTexCoord);
}