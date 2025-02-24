#version 450

layout(location = 0) out vec4 outColor;

layout(location = 1) in vec2 fragUV;

layout(binding = 0) uniform sampler2D tex;

void main()
{
    outColor = texture(tex, fragUV);
}