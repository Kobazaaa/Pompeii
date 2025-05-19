#version 450 core
#extension GL_GOOGLE_include_directive : require

// -- Includes --
#include "helpers.glsl"

// -- Data --
layout(set = 0, binding = 0) uniform sampler2D Render;
layout(set = 0, binding = 1) uniform CameraSettings
{
	float aperture;
	float shutterspeed;
	float iso;
} camSettings;

// -- Input --
layout(location = 0) in vec2 fragTexCoord;

// -- Output --
layout(location = 0) out vec4 outColor;

// -- Shader --
void main()
{
	// -- Base Color --
	vec3 color = texture(Render, fragTexCoord).rgb;
	
	// -- Tone Mapping --
	vec3 aces = ACESFilmToneMapping(color);
	vec3 reinhard = ReinhardToneMapping(color);
	vec3 uncharted2 = Uncharted2ToneMapping(color);
	color = aces;

	// -- Output --
	outColor = vec4(color, 1.0);
}
