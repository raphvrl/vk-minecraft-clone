#ifndef GLOBAL_GLSL
#define GLOBAL_GLSL

#extension GL_EXT_nonuniform_qualifier : require

#define CAMERA_UBO_IDX 0

struct CameraUBO {
    mat4 view;
    mat4 proj;
    mat4 ortho;
    vec3 position;
};

layout(binding = 0) uniform UBOArray {
    CameraUBO camera;
} uboArray[];

layout(binding = 1) uniform SSBOArray {
} ssboArray[];

layout(binding = 2) uniform sampler2D texArr[];

#endif // GLOBAL_GLSL