#version 450 core
#extension GL_GOOGLE_include_directive : require

// -- Includes --
#include "constants.glsl"
#include "helpers_general.glsl"

// -- Input --
layout(location = 0) in vec3 fragLocalPos;
layout(set = 0, binding = 0) uniform samplerCube hdri;

// -- Output --
layout(location = 0) out vec4 outColor;

// -- Shader --
void main()
{
	// Sample dir == hemisphere orientation
	vec3 normal = normalize(fragLocalPos);
	vec3 tangent;
	vec3 bitangent;
	CalculateTangents(normal, tangent, bitangent);

	vec3 irradiance = vec3(0.0);
	int sampleCount = 0;
	const float sampleDelta = 0.025;
	for(float phi = 0.0; phi < TWO_PI; phi += sampleDelta)
	{
		for(float theta = 0.0; theta < HALF_PI; theta += sampleDelta)
		{
			// sperical to cart. (tangent space)
			vec3 tangentSample = vec3(
			sin(theta) * cos(phi),
			sin(theta) * sin(phi),
			cos(theta));

			// tagent to world
			vec3 sampleVec = 
				tangentSample.x * tangent +
				tangentSample.y * bitangent + 
				tangentSample.z * normal;

			// sample and accum
			irradiance += texture(hdri, sampleVec).rgb * cos(theta) * sin(theta);
			++sampleCount;
		}
	}

	// Integrate!!!
	irradiance = PI * irradiance * (1.0 / float(sampleCount));
	outColor = vec4(irradiance, 1.0);
}
