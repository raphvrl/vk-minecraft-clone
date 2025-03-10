#version 450

layout(location = 0) out vec4 outColor;

layout(location = 1) in vec3 camPos;
layout(location = 2) in vec3 worldPos;

vec4 addFog(vec4 color, float dist)
{
    float fogStart = 7.0 * 16.0;
    float fogEnd = 9.0 * 16.0;
    vec3 fogColor = vec3(0.73, 0.83, 1.0);

    float fogFactor = 1.0 - clamp(
        (fogEnd - dist) / (fogEnd - fogStart),
        0.0,
        1.0
    );

    return vec4(mix(color.rgb, fogColor, fogFactor), color.a);
} 

void main()
{
    float dist = length(worldPos - camPos);

    vec4 color = vec4(1.0, 1.0, 1.0, 0.7);
    color = addFog(color, dist);

    outColor = color;
}