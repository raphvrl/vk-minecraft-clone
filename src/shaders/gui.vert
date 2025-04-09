#version 450
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require

#include "global.glsl"

const vec2 positions[] = vec2[6](
    vec2(0.0, 0.0),
    vec2(1.0, 0.0),
    vec2(0.0, 1.0),
    
    vec2(0.0, 1.0),
    vec2(1.0, 0.0),
    vec2(1.0, 1.0)
);

layout(binding = 0) uniform UBOArray {
    CameraUBO camera;
} uboArray[];

layout(push_constant) uniform PushConstantsObject {
    mat4 model;
    vec4 uv;
    uint textureID;
} pco;

layout(location = 0) out vec2 fragUV;

void main()
{
    mat4 ortho = uboArray[CAMERA_UBO_IDX].camera.ortho;
    mat4 model = pco.model;

    vec2 pos = positions[gl_VertexIndex];
    gl_Position = ortho * model * vec4(pos, 0.0, 1.0);

    fragUV = mix(pco.uv.xy, pco.uv.zw, pos);
    fragUV.y = 1.0 - fragUV.y;
}