#version 450

layout(binding = 0) uniform UniformBufferObjectCamera {
    mat4 view;
    mat4 model;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 wPos;

void main() {
    gl_Position =  ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    wPos = gl_Position.xyz;
    fragColor = inColor;
}