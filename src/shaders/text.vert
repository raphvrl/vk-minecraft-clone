#version 450

const vec2 positions[6] = vec2[](
    vec2(0.0, 0.0),
    vec2(1.0, 0.0),
    vec2(0.0, 1.0),

    vec2(0.0, 1.0),
    vec2(1.0, 0.0),
    vec2(1.0, 1.0)
);

layout(push_constant) uniform PushConstants {
    vec2 pos;
    vec2 uv;
    float size;
} pco;

layout(location = 0) out vec2 fragUV;

void main()
{
    float aspectRatio = 16.0 / 9.0; 

    vec2 scaledPos = vec2(
        positions[gl_VertexIndex].x * pco.size,
        positions[gl_VertexIndex].y * pco.size * aspectRatio
    );

    vec2 finalPos = pco.pos + scaledPos;
    
    gl_Position = vec4(finalPos, 0.0, 1.0);
    
    fragUV = pco.uv + positions[gl_VertexIndex] * (1.0/16.0);
    fragUV.y = 1.0 - fragUV.y;
}