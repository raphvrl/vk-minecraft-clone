#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec2 fragUV;
layout(location = 1) out vec3 camPos;
layout(location = 2) out vec3 worldPos;

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec3 camPos;
} ubo;

layout(push_constant) uniform PushConstantObject {
    mat4 model;
} pco;

void main()
{

    fragUV = inUV;
    camPos = ubo.camPos;
    worldPos = (pco.model * vec4(inPos, 1.0)).xyz;
    gl_Position = ubo.proj * ubo.view * pco.model * vec4(inPos, 1.0);
}