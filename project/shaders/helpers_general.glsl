#ifndef HELPER_GENERAL
#define HELPER_GENERAL
#include "constants.glsl"

// -- World Position --
vec3 GetWorldPositionFromDepth(in float depth, in ivec2 fragCoords, in vec2 resolution, in mat4 invProj, in mat4 invView)
{
	vec2 ndc = vec2(
					(float(fragCoords.x) / resolution.x) * 2.0 - 1.0,
					(float(fragCoords.y) / resolution.y) * 2.0 - 1.0
				   );
//	ndc.y *= -1.0;
	const vec4 clipPos = vec4(ndc, depth, 1.0);

	vec4 viewPos = invProj * clipPos;
	viewPos /= viewPos.w;

	vec4 worldPos = invView * viewPos;
	return worldPos.xyz;
}

// -- Tangent To World --
void CalculateTangents(in vec3 N, out vec3 T, out vec3 B)
{
	vec3 normal = normalize(N);
	vec3 up = abs(normal.y) < (1.0 - EPSILON) ? vec3(0.0, 1.0, 0.0) : vec3(0.0, 0.0, -1.0);
	T = normalize(cross(normal, up));
	B = normalize(cross(T, normal));
}

#endif //HELPER_GENERAL
