#version 450
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require

#include "global.glsl"

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec2 fragUV;

layout(binding = 2) uniform sampler2D texArr[];

layout(push_constant) uniform PushConstantsObject {
    uint textureID;
    vec4 color;
} pco;

void main()
{
    vec3 color = texture(texArr[pco.textureID], fragUV).rgb;
    float alpha = texture(texArr[pco.textureID], fragUV).a;

    outColor = vec4(color, alpha) * pco.color;
}