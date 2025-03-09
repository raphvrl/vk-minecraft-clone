#version 450

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D tex;

layout(location = 0) in vec2 fragUV;

void main()
{
    vec3 color = texture(tex, fragUV).rgb;
    float alpha = texture(tex, fragUV).a;

    outColor = vec4(color, alpha);
}