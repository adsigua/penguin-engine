#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 pos;
layout(location = 2) in vec4 wPos;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(pos, 1.0);
}