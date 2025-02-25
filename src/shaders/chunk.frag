#version 450

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec2 fragUV;
layout(location = 1) in vec3 camPos;
layout(location = 2) in vec3 worldPos;

layout(set = 0, binding = 1) uniform sampler2D tex; 

vec4 addFog(vec4 color, float dist)
{
    float fogStart = 4.0 * 16.0;
    float fogEnd = 6.5 * 16.0;
    vec3 fogColor = vec3(0.62, 0.84, 1.0);

    float fogFactor = 1.0 - clamp(
        (fogEnd - dist) / (fogEnd - fogStart),
        0.0,
        1.0
    );

    fogFactor = fogFactor * fogFactor;

    if (dist > 8.0 * 16.0 * 0.95) {
        fogFactor = 1.0;
    }

    return vec4(mix(color.rgb, fogColor, fogFactor), color.a);
} 

void main()
{
    float dist = length(worldPos - camPos);

    vec4 color = texture(tex, fragUV);
    color = addFog(color, dist);

    outColor = color;
}