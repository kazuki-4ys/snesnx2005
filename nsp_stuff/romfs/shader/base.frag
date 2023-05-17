#version 330 core

in vec4 ourColor;

out vec4 fragColor;

void main(){
    fragColor = ourColor;
    //fragColor = vec4(1.0, 0, 1.0, 1.0);
}