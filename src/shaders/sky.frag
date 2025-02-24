#version 450

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 fragPos;

void main()
{
    vec3 skyColor = vec3(0.47, 0.65, 1.0);
    vec3 horizonColor = vec3(0.73, 0.84, 1.0);

    vec3 normalizedPos = normalize(fragPos);

    float horizontalDist = length(vec2(normalizedPos.x, normalizedPos.z));
    float heightFactor = normalizedPos.y;
    
    float factor = heightFactor + (1.0 - horizontalDist) * 0.3;
    factor = clamp(factor, 0.0, 1.0);
    
    factor = smoothstep(0.0, 0.15, factor);

    vec3 color = mix(horizonColor, skyColor, factor);
    outColor = vec4(color, 1.0);
}