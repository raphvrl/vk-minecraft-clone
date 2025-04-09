#ifndef GLOBAL_GLSL
#define GLOBAL_GLSL

#define CAMERA_UBO_IDX 0

struct CameraUBO {
    mat4 view;
    mat4 proj;
    mat4 ortho;
    vec3 position;
};

#endif // GLOBAL_GLSL