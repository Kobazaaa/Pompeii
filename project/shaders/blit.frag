#version 450

// -- Data --
layout(set = 0, binding = 0) uniform sampler2D Render;

// -- Input --
layout(location = 0) in vec2 fragTexCoord;

// -- Output --
layout(location = 0) out vec4 outColor;

// -- Shader --
void main()
{
	// -- Ugly Magenta --
	outColor = vec4(1.0, 0.0, 1.0, 1.0);
	
	// -- Blit --
	outColor = texture(Render, fragTexCoord);
}
