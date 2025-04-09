#version 450
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require

#include "global.glsl"

const float outlineOffset = 0.002;
const vec3 positions[] = vec3[24](
    // Front face
    vec3(-outlineOffset, -outlineOffset, -outlineOffset),
    vec3(1.0 + outlineOffset, -outlineOffset, -outlineOffset),
    vec3(1.0 + outlineOffset, -outlineOffset, -outlineOffset),
    vec3(1.0 + outlineOffset, 1.0 + outlineOffset, -outlineOffset),
    vec3(1.0 + outlineOffset, 1.0 + outlineOffset, -outlineOffset),
    vec3(-outlineOffset, 1.0 + outlineOffset, -outlineOffset),
    vec3(-outlineOffset, 1.0 + outlineOffset, -outlineOffset),
    vec3(-outlineOffset, -outlineOffset, -outlineOffset),

    // Back face
    vec3(-outlineOffset, -outlineOffset, 1.0 + outlineOffset),
    vec3(1.0 + outlineOffset, -outlineOffset, 1.0 + outlineOffset),
    vec3(1.0 + outlineOffset, -outlineOffset, 1.0 + outlineOffset),
    vec3(1.0 + outlineOffset, 1.0 + outlineOffset, 1.0 + outlineOffset),
    vec3(1.0 + outlineOffset, 1.0 + outlineOffset, 1.0 + outlineOffset),
    vec3(-outlineOffset, 1.0 + outlineOffset, 1.0 + outlineOffset),
    vec3(-outlineOffset, 1.0 + outlineOffset, 1.0 + outlineOffset),
    vec3(-outlineOffset, -outlineOffset, 1.0 + outlineOffset),

    // In-between
    vec3(-outlineOffset, -outlineOffset, -outlineOffset),
    vec3(-outlineOffset, -outlineOffset, 1.0 + outlineOffset),
    vec3(1.0 + outlineOffset, -outlineOffset, -outlineOffset),
    vec3(1.0 + outlineOffset, -outlineOffset, 1.0 + outlineOffset),
    vec3(1.0 + outlineOffset, 1.0 + outlineOffset, -outlineOffset),
    vec3(1.0 + outlineOffset, 1.0 + outlineOffset, 1.0 + outlineOffset),
    vec3(-outlineOffset, 1.0 + outlineOffset, -outlineOffset),
    vec3(-outlineOffset, 1.0 + outlineOffset, 1.0 + outlineOffset)
);

layout(binding = 0) uniform UBOArray {
    CameraUBO camera;
} uboArray[];

layout(push_constant) uniform PushConstantObject {
    mat4 model;
} pco;

void main()
{
    mat4 view = uboArray[CAMERA_UBO_IDX].camera.view;
    mat4 proj = uboArray[CAMERA_UBO_IDX].camera.proj;
    mat4 model = pco.model;

    gl_Position = proj * view * model * vec4(positions[gl_VertexIndex], 1.0);
}