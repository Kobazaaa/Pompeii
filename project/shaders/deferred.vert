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
layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec3 fragTangent;
layout(location = 3) out vec3 fragBitangent;
layout(location = 4) out vec2 fragTexCoord;
layout(location = 5) out vec3 fragWorldPos;

// -- Shader --
void main()
{
    gl_Position			= ubo.proj * ubo.view * modelData.model * vec4(inPosition, 1.0);
	fragColor			= inColor;
	fragNormal			= normalize(mat3(modelData.model) * inNormal);
	fragTangent			= normalize(mat3(modelData.model) * inTangent);
	fragBitangent		= normalize(mat3(modelData.model) * inBitangent);
	fragTexCoord		= inTexCoord;
	fragWorldPos		= (modelData.model * vec4(inPosition, 1.0)).rgb;
}