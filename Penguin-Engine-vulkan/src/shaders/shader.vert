#version 450

layout(binding = 0) uniform UniformBufferObjectCamera {
    mat4 view;
    mat4 proj;
} ubo;

layout(binding = 2) uniform UniformBufferObjectModel {
    mat4 model;
} ubom;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 pos;
layout(location = 3) out vec4 wPos;


void main() {
    wPos = ubom.model * vec4(inPosition, 1.0);
    gl_Position =  ubo.proj * ubo.view * wPos;
    fragTexCoord = inTexCoord;
    pos = inPosition;
    fragColor = inColor;
}