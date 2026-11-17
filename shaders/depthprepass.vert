#version 450

// -- Matrices --
layout(set = 0, binding = 0) uniform MatrixUBO
{
	mat4 view;
	mat4 proj;
} ubo;

// -- Model Data --
layout(push_constant) uniform constants
{
	mat4 model;
} modelData;


// -- Input --
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inBitangent;
layout(location = 4) in vec3 inColor;
layout(location = 5) in vec2 inTexCoord;

// -- Output --
layout(location = 0) out vec2 fragTexCoord;

// -- Shader --
void main()
{
    gl_Position = ubo.proj * ubo.view * modelData.model * vec4(inPosition, 1.0);
	fragTexCoord = inTexCoord;
}