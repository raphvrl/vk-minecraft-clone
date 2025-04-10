#version 450
#extension GL_GOOGLE_include_directive : require

#include "binding.glsl"

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec2 fragUV;

layout(push_constant) uniform PushConstantsObject {
    uint textureID;
} pco;

void main()
{
    vec3 color = texture(texArr[pco.textureID], fragUV).rgb;
    float alpha = texture(texArr[pco.textureID], fragUV).a;

    alpha = min(alpha, 0.3);

    outColor = vec4(color, alpha);
}