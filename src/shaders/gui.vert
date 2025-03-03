#version 450

const vec2 positions[] = vec2[6](
    vec2(0.0, 0.0),
    vec2(1.0, 0.0),
    vec2(0.0, 1.0),
    
    vec2(0.0, 1.0),
    vec2(1.0, 0.0),
    vec2(1.0, 1.0)
);

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 ortho;
} ubo;

layout(push_constant) uniform PushConstantsObject {
    mat4 model;
    vec4 uv;
} pco;

layout(location = 0) out vec2 fragUV;

void main()
{
    vec2 pos = positions[gl_VertexIndex];
    gl_Position = ubo.ortho * pco.model * vec4(pos, 0.0, 1.0);

    fragUV = mix(pco.uv.xy, pco.uv.zw, pos);
    fragUV.y = 1.0 - fragUV.y;
}