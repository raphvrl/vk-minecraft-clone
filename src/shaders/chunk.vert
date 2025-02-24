#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUV;

layout (location = 0) out vec2 fragUV;

layout(set = 0, binding = 0) uniform UniformBufferObject
{
    mat4 viewProj;
} ubo;

layout(push_constant) uniform PushConstantObject
{
    mat4 model;
    mat4 viewPort;
} pco;

void main()
{
    fragUV = inUV;
    gl_Position = pco.viewPort * pco.model * vec4(inPos, 1.0);
}