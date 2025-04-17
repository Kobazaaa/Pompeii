#version 450

// -- UBO --
layout(set = 0, binding = 0) uniform UniformBufferObject
{
	mat4 model;
	mat4 view;
	mat4 proj;
	vec3 cam;
} ubo;


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
layout(location = 5) out vec3 fragViewDir;

// -- Shader --
void main()
{
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
	fragColor = inColor;
	fragNormal = normalize(mat3(ubo.model) * inNormal);
	fragTangent = normalize(mat3(ubo.model) * inTangent);
	fragBitangent = normalize(mat3(ubo.model) * inBitangent);
	fragTexCoord = inTexCoord;
	fragViewDir = normalize(vec3(ubo.model * vec4(inPosition, 1.0)) - ubo.cam);
}