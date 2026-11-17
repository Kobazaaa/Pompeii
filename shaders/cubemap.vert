#version 450

// Hardcoded cube vertices for 3D unit Cube (6 faces * 2 triangles * 3 vertices = 36)
const vec3 positions[36] = vec3[36]
(
    // +X
    vec3( 1,  1, -1), vec3( 1, -1, -1), vec3( 1, -1, 1),
    vec3( 1,  1, -1), vec3( 1, -1,  1), vec3( 1,  1, 1),

    // -X
    vec3(-1, -1, -1), vec3(-1,  1, -1), vec3(-1,  1, 1),
    vec3(-1, -1, -1), vec3(-1,  1,  1), vec3(-1, -1, 1),

    // +Y
    vec3(-1,  1, -1), vec3( 1,  1, -1), vec3( 1,  1, 1),
    vec3(-1,  1, -1), vec3( 1,  1,  1), vec3(-1,  1, 1),

    // -Y
    vec3(-1, -1,  1), vec3( 1, -1,  1), vec3( 1, -1, -1),
    vec3(-1, -1,  1), vec3( 1, -1, -1), vec3(-1, -1, -1),

    // +Z
    vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1),
    vec3( 1, -1,  1), vec3(-1,  1,  1), vec3( 1,  1,  1),

    // -Z
    vec3(-1, -1, -1), vec3( 1, -1, -1), vec3( 1,  1, -1),
    vec3(-1, -1, -1), vec3( 1,  1, -1), vec3(-1,  1, -1)
);

// -- Input PC --
layout(push_constant) uniform PushConstants {
    mat4 view;
    mat4 proj;
} pc;

// -- Output --
layout(location = 0) out vec3 fragLocalPos;

// -- Shader --
void main()
{
    vec3 pos = positions[gl_VertexIndex];
    fragLocalPos = pos;
    gl_Position = pc.proj * pc.view * vec4(pos, 1.0);
}
