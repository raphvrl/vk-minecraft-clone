#version 450

layout(location = 0) out vec4 outColor;

void main()
{
    vec2 uv = gl_FragCoord.xy / vec2(1280.0, 720.0);

    vec3 skyColor = vec3(0.45, 0.71, 1.0);

    float t = uv.y;
    vec3 finalColor = mix(skyColor, skyColor, t);  

    outColor = vec4(finalColor, 1.0);
}