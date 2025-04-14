#version 450
#extension GL_GOOGLE_include_directive : require

#include "binding.glsl"

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec2 fragUV;
layout(location = 1) in flat uint faceIndex;

layout(push_constant) uniform PushConstant
{
    mat4 model;
    vec2 topUV;
    vec2 bottomUV;
    vec2 sideUV;
    uint textureID;
} pc;

void main()
{
    vec3 color = texture(texArr[pc.textureID], fragUV).rgb;
    float alpha = texture(texArr[pc.textureID], fragUV).a;

    if (alpha < 0.1) {
        discard;
    }

    float faceLights[6] = float[](
        0.6, 0.6, 0.8, 0.8, 1.0, 1.0
    );

    float light = faceLights[faceIndex];
    color = color * light;

    outColor = vec4(color, alpha);
}