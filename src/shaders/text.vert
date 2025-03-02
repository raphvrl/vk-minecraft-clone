#version 450

const vec2 positions[6] = vec2[](
    vec2(0.0, 0.0),
    vec2(1.0, 0.0),
    vec2(0.0, 1.0),
    
    vec2(0.0, 1.0),
    vec2(1.0, 0.0),
    vec2(1.0, 1.0)
);

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 proj;
} ubo;

layout(push_constant) uniform PushConstantsObject {
    mat4 model;
    vec2 uv;
    vec4 color;
} pco;

layout(location = 0) out vec2 fragUV;
layout(location = 1) out vec4 fragColor;

void main()
{
    gl_Position = ubo.proj * pco.model * vec4(positions[gl_VertexIndex], 0.0, 1.0);

    fragUV = pco.uv + positions[gl_VertexIndex] * (1.0 / 16.0);
    fragUV.y = 1.0 - fragUV.y;

    fragColor = pco.color;
}