#version 450
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require

#include "global.glsl"

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inUV;
layout(location = 2) in uint inLightLevel;
layout(location = 3) in uint inFaceDirection;

layout(location = 0) out vec2 fragUV;
layout(location = 1) out vec3 camPos;
layout(location = 2) out vec3 worldPos;
layout(location = 3) out flat uint lightLevel;
layout(location = 4) out flat uint faceDirection;

layout(set = 0, binding = 0) uniform UBOArray {
    CameraUBO camera;
} uboArray[];

layout(push_constant) uniform PushConstantObject {
    mat4 model;
    uint textureId;
} pco;

void main()
{
    fragUV = inUV;
    camPos = uboArray[CAMERA_UBO_IDX].camera.position;
    lightLevel = inLightLevel;
    faceDirection = inFaceDirection;
    
    mat4 view = uboArray[CAMERA_UBO_IDX].camera.view;
    mat4 proj = uboArray[CAMERA_UBO_IDX].camera.proj;

    worldPos = (pco.model * vec4(inPos, 1.0)).xyz;
    gl_Position = proj * view * vec4(worldPos, 1.0);
}