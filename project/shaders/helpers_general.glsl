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
