#version 450

layout(location = 0) out vec4 outColor;

layout(location = 1) in vec3 camPos;
layout(location = 2) in vec3 worldPos;

const float CLOUD_SIZE = 12.0;

vec4 addFog(vec4 color, float dist)
{
    float fogStart = 8.0 * CLOUD_SIZE;
    float fogEnd = 16.0 * CLOUD_SIZE;
    vec3 fogColor = vec3(0.62, 0.84, 1.0);

    float fogFactor = 1.0 - clamp(
        (fogEnd - dist) / (fogEnd - fogStart),
        0.0,
        1.0
    );

    fogFactor = fogFactor * fogFactor;

    return vec4(mix(color.rgb, fogColor, fogFactor), color.a);
} 

void main()
{
    float dist = length(worldPos - camPos);

    vec4 color = vec4(1.0, 1.0, 1.0, 1.0);
    color = addFog(color, dist);

    outColor = color;
}