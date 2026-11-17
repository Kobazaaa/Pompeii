#version 450

// -- Model Data --
layout(push_constant) uniform PushConstants
{
	mat4 projView;
	mat4 model;
} pc;


// -- Input --
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inBitangent;
layout(location = 4) in vec3 inColor;
layout(location = 5) in vec2 inTexCoord;

// -- Shader --
void main()
{
    gl_Position = pc.projView * pc.model * vec4(inPosition, 1.0);
}