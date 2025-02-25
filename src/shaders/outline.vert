#version 450

const float scale = 1.005;

const vec3 positions[] = vec3[24](
    vec3(0.0, 0.0, 0.0) * scale,
    vec3(1.0, 0.0, 0.0) * scale,
    vec3(1.0, 0.0, 0.0) * scale,
    vec3(1.0, 1.0, 0.0) * scale,
    vec3(1.0, 1.0, 0.0) * scale,
    vec3(0.0, 1.0, 0.0) * scale,
    vec3(0.0, 1.0, 0.0) * scale,
    vec3(0.0, 0.0, 0.0) * scale,

    vec3(0.0, 0.0, 1.0) * scale,
    vec3(1.0, 0.0, 1.0) * scale,
    vec3(1.0, 0.0, 1.0) * scale,
    vec3(1.0, 1.0, 1.0) * scale,
    vec3(1.0, 1.0, 1.0) * scale,
    vec3(0.0, 1.0, 1.0) * scale,
    vec3(0.0, 1.0, 1.0) * scale,
    vec3(0.0, 0.0, 1.0) * scale,

    vec3(0.0, 0.0, 0.0) * scale,
    vec3(0.0, 0.0, 1.0) * scale,
    vec3(1.0, 0.0, 0.0) * scale,
    vec3(1.0, 0.0, 1.0) * scale,
    vec3(1.0, 1.0, 0.0) * scale,
    vec3(1.0, 1.0, 1.0) * scale,
    vec3(0.0, 1.0, 0.0) * scale,
    vec3(0.0, 1.0, 1.0) * scale
);

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

void main()
{
    mat4 mvp = ubo.proj * ubo.view * ubo.model;
    gl_Position = mvp * vec4(positions[gl_VertexIndex], 1.0);
}