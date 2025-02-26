#version 450

const float outlineOffset = 0.002;
const vec3 positions[] = vec3[24](
    vec3(-outlineOffset, -outlineOffset, -outlineOffset),
    vec3(1.0 + outlineOffset, -outlineOffset, -outlineOffset),
    vec3(1.0 + outlineOffset, -outlineOffset, -outlineOffset),
    vec3(1.0 + outlineOffset, 1.0 + outlineOffset, -outlineOffset),
    vec3(1.0 + outlineOffset, 1.0 + outlineOffset, -outlineOffset),
    vec3(-outlineOffset, 1.0 + outlineOffset, -outlineOffset),
    vec3(-outlineOffset, 1.0 + outlineOffset, -outlineOffset),
    vec3(-outlineOffset, -outlineOffset, -outlineOffset),

    vec3(-outlineOffset, -outlineOffset, 1.0 + outlineOffset),
    vec3(1.0 + outlineOffset, -outlineOffset, 1.0 + outlineOffset),
    vec3(1.0 + outlineOffset, -outlineOffset, 1.0 + outlineOffset),
    vec3(1.0 + outlineOffset, 1.0 + outlineOffset, 1.0 + outlineOffset),
    vec3(1.0 + outlineOffset, 1.0 + outlineOffset, 1.0 + outlineOffset),
    vec3(-outlineOffset, 1.0 + outlineOffset, 1.0 + outlineOffset),
    vec3(-outlineOffset, 1.0 + outlineOffset, 1.0 + outlineOffset),
    vec3(-outlineOffset, -outlineOffset, 1.0 + outlineOffset),

    vec3(-outlineOffset, -outlineOffset, -outlineOffset),
    vec3(-outlineOffset, -outlineOffset, 1.0 + outlineOffset),
    vec3(1.0 + outlineOffset, -outlineOffset, -outlineOffset),
    vec3(1.0 + outlineOffset, -outlineOffset, 1.0 + outlineOffset),
    vec3(1.0 + outlineOffset, 1.0 + outlineOffset, -outlineOffset),
    vec3(1.0 + outlineOffset, 1.0 + outlineOffset, 1.0 + outlineOffset),
    vec3(-outlineOffset, 1.0 + outlineOffset, -outlineOffset),
    vec3(-outlineOffset, 1.0 + outlineOffset, 1.0 + outlineOffset)
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