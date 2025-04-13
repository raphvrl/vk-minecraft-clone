#version 450
#extension GL_GOOGLE_include_directive : require

#include "binding.glsl"

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec2 fragUV;
layout(location = 1) in vec4 fragColor;

layout(push_constant) uniform PushConstantsObject {
    mat4 model;
    vec2 uv;
    vec4 color;
    uint textureID;
} pco;

void main()
{
    outColor = texture(texArr[pco.textureID], fragUV) * fragColor;
}