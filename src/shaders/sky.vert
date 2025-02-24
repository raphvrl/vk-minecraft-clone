#version 450

layout(location = 0) in vec3 inPos;

layout(push_constant) uniform PushConstants {
    mat4 viewProj;
} cam;

layout(location = 0) out vec3 fragPos;

void main()
{
    fragPos = inPos;

    vec4 pos = cam.viewProj * vec4(inPos, 1.0);
    gl_Position = pos.xyww;
}