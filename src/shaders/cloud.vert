#version 450

vec3 positions[] = vec3[6](
    vec3(-1.0, 0.0, -1.0),
    vec3(-1.0, 0.0,  1.0),
    vec3( 1.0, 0.0,  1.0),

    vec3(-1.0, 0.0, -1.0),
    vec3( 1.0, 0.0,  1.0),
    vec3( 1.0, 0.0, -1.0)
);

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
    vec3 pos = positions[gl_VertexIndex] * 12.0;
    camPos = ubo.camPos;
    worldPos = (pco.model * vec4(pos, 1.0)).xyz;
    gl_Position = ubo.proj * ubo.view * pco.model * vec4(pos, 1.0);
}