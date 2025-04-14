#version 450
#extension GL_GOOGLE_include_directive : require

#include "binding.glsl"

// cube pos

const vec3 positions[36] = vec3[](
    vec3(-0.5, -0.5, 0.5), vec3(0.5, -0.5, 0.5), vec3(0.5, 0.5, 0.5),
    vec3(0.5, 0.5, 0.5), vec3(-0.5, 0.5, 0.5), vec3(-0.5, -0.5, 0.5),

    vec3(0.5, -0.5, -0.5), vec3(-0.5, -0.5, -0.5), vec3(-0.5, 0.5, -0.5),
    vec3(-0.5, 0.5, -0.5), vec3(0.5, 0.5, -0.5), vec3(0.5, -0.5, -0.5),

    vec3(-0.5, -0.5, -0.5), vec3(-0.5, -0.5, 0.5), vec3(-0.5, 0.5, 0.5),
    vec3(-0.5, 0.5, 0.5), vec3(-0.5, 0.5, -0.5), vec3(-0.5, -0.5, -0.5),

    vec3(0.5, -0.5, 0.5), vec3(0.5, -0.5, -0.5), vec3(0.5, 0.5, -0.5),
    vec3(0.5, 0.5, -0.5), vec3(0.5, 0.5, 0.5), vec3(0.5, -0.5, 0.5),

    vec3(-0.5, 0.5, 0.5), vec3(0.5, 0.5, 0.5), vec3(0.5, 0.5, -0.5),
    vec3(0.5, 0.5, -0.5), vec3(-0.5, 0.5, -0.5), vec3(-0.5, 0.5, 0.5),

    vec3(-0.5, -0.5, -0.5), vec3(0.5, -0.5, -0.5), vec3(0.5, -0.5, 0.5),
    vec3(0.5, -0.5, 0.5), vec3(-0.5, -0.5, 0.5), vec3(-0.5, -0.5, -0.5)
);

const vec2 uvs[6] = vec2[](
    vec2(1.0, 0.0), vec2(0.0, 0.0), vec2(0.0, 1.0),
    vec2(0.0, 1.0), vec2(1.0, 1.0), vec2(1.0, 0.0)
);

layout(location = 0) out vec2 fragUV;
layout(location = 1) out uint faceIndex;

layout(push_constant) uniform PushConstant
{
    mat4 model;
    vec2 topUV;
    vec2 bottomUV;
    vec2 sideUV;
    uint textureID;
} pc;

void main()
{
    int faceidx = gl_VertexIndex / 6;
    int vertexIndex = gl_VertexIndex % 6;

    vec3 pos = positions[gl_VertexIndex];
    vec2 uv = uvs[vertexIndex];

    mat4 ortho = uboArray[CAMERA_UBO_IDX].camera.ortho;
    mat4 model = pc.model;

    gl_Position = ortho * model * vec4(pos, 1.0);

    vec2 tileUV;
    
    if (faceidx == 0) {
        tileUV = pc.sideUV;
    } else if (faceidx == 1) {
        tileUV = pc.sideUV;
    } else if (faceidx == 2) {
        tileUV = pc.sideUV;
    } else if (faceidx == 3) {
        tileUV = pc.sideUV;
    } else if (faceidx == 4) {
        tileUV = pc.topUV;
    } else {
        tileUV = pc.bottomUV;
    }

    const float TILE_SIZE = 1.0 / 16.0;

    fragUV = vec2(
        (tileUV.x + uv.x) * TILE_SIZE,
        (tileUV.y + uv.y) * TILE_SIZE
    );

    fragUV.y = 1.0 - fragUV.y;

    faceIndex = uint(faceidx);
}