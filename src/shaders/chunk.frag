#version 450
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require

#include "global.glsl"

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec2 fragUV;
layout(location = 1) in vec3 camPos;
layout(location = 2) in vec3 worldPos;
layout(location = 3) in flat uint lightLevel;
layout(location = 4) in flat uint faceDirection;

layout(binding = 2) uniform sampler2D texArr[];

layout(push_constant) uniform PushConstantObject {
    mat4 model;
    uint textureId;
} pco;

vec3 addShadow(vec3 color)
{
    float faceLight = 1.0;
    switch (faceDirection) {
        case 0: faceLight = 0.7; break;
        case 1: faceLight = 0.7; break;
        case 2: faceLight = 0.8; break;
        case 3: faceLight = 0.8; break;
        case 4: faceLight = 1.0; break;
        case 5: faceLight = 0.5; break;
    }

    float intensityFactors[16] = float[](
        0.01, 0.02, 0.04, 0.07,
        0.11,  0.16, 0.22, 0.29,
        0.36,  0.44, 0.52, 0.61,
        0.70,  0.78, 0.88, 1.0
    );

    float light = intensityFactors[lightLevel];

    float ambientLight = 0.05;
    vec3 litColor = mix(color * ambientLight, color * faceLight, light);

    return litColor;
}

vec3 addFog(vec3 color, float dist)
{
    float fogStart = 4.0 * 16.0;
    float fogEnd = 7.0 * 16.0;
    vec3 fogColor = vec3(0.73, 0.83, 1.0);

    float fogFactor = 1.0 - clamp(
        (fogEnd - dist) / (fogEnd - fogStart),
        0.0,
        1.0
    );

    fogFactor = pow(fogFactor, 0.7);

    return mix(color, fogColor, fogFactor);
}

void main()
{
    float dist = length(worldPos - camPos);

    vec3 color = texture(texArr[pco.textureId], fragUV).rgb;
    float alpha = texture(texArr[pco.textureId], fragUV).a;

    if (alpha > 0.1 && alpha < 0.9) {
        alpha = 0.8;
    }

    color = addShadow(color);

    color = addFog(color, dist);


    outColor = vec4(color, alpha);
}