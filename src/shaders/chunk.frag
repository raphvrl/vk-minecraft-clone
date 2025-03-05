#version 450

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec2 fragUV;
layout(location = 1) in vec3 camPos;
layout(location = 2) in vec3 worldPos;
layout(location = 3) in flat uint lightLevel;

layout(set = 0, binding = 1) uniform sampler2D tex; 

vec4 addFog(vec4 color, float dist)
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

    return vec4(mix(color.rgb, fogColor, fogFactor), color.a);
} 

void main()
{
    float dist = length(worldPos - camPos);

    vec4 color = texture(tex, fragUV);

    float light = float(lightLevel) / 15.0;
    color.rgb *= light;

    color = addFog(color, dist);

    outColor = color;
}