#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUV;

layout (location = 1) out vec2 fragUV;

layout(push_constant) uniform PushConstants {
    mat4 mvp;
} camera;

void main() {
    fragUV = inUV;
    gl_Position = camera.mvp * vec4(inPos, 1.0);
}