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
layout(set = 0, binding = 2) uniform sampler2D AverageLum;

// -- Input --
layout(location = 0) in vec2 fragTexCoord;

// -- Output --
layout(location = 0) out vec4 outColor;

// -- Shader --
void main()
{
	// -- Base Color --
	vec3 ldrColor = texture(Render, fragTexCoord).rgb;
	
	// -- Camera Exposure --
	const float EV100 = CalculateEV100(camSettings.aperture, camSettings.shutterspeed, camSettings.iso);
	const float exposure = EV100ToExposure(EV100);

	const float autoEV100 = AverageLuminanceToEV100(texelFetch(AverageLum, ivec2(0,0), 0).x);
	const float autoExposure = EV100ToExposure(autoEV100);
	ldrColor *= autoExposure;

	// -- Tone Mapping --
	const vec3 aces = ACESFilmToneMapping(ldrColor);
	const vec3 reinhard = ReinhardToneMapping(ldrColor);
	const vec3 uncharted2 = Uncharted2ToneMapping(ldrColor);
	ldrColor = uncharted2;

	// -- Output --
	outColor = vec4(ldrColor, 1.0);
}
