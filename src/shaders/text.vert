#version 450
#extension GL_GOOGLE_include_directive : require

#include "binding.glsl"

const vec2 positions[6] = vec2[](
    vec2(0.0, 0.0),
    vec2(1.0, 0.0),
    vec2(0.0, 1.0),
    
    vec2(0.0, 1.0),
    vec2(1.0, 0.0),
    vec2(1.0, 1.0)
);

layout(push_constant) uniform PushConstantsObject {
    mat4 model;
    vec2 uv;
    vec4 color;
    uint textureID;
} pco;

layout(location = 0) out vec2 fragUV;
layout(location = 1) out vec4 fragColor;

void main()
{
    mat4 ortho = uboArray[CAMERA_UBO_IDX].camera.ortho;
    mat4 model = pco.model;

    vec2 pos = positions[gl_VertexIndex];
    gl_Position = ortho * model * vec4(pos, 0.0, 1.0);

    fragUV = pco.uv + positions[gl_VertexIndex] * (1.0 / 16.0);
    fragUV.y = 1.0 - fragUV.y;

    fragColor = pco.color;
}