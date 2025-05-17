// -- Constants --
float PI = 3.1415926535897932384626433832795028841971693993751058209;


// -- Normal Distrbution Function --
float ThrowbridgeReitzGGX(in float a, in vec3 n, in vec3 h)
{
	float a2 = a * a;
	float nDoth = max(dot(n, h), 0.0);
	return a2 / (PI * pow((nDoth * nDoth) * (a2 -1) + 1, 2));
}

// -- Fresnel Schlick Function --
vec3 FresnelSchlick(in vec3 h, in vec3 v, in vec3 F0)
{
	float hDotv = max(dot(h, v), 0.0);
	return F0 + (1 - F0) * pow(1 - hDotv, 5);
}

// -- Geometry Distribution Function --
float GeometrySchlickGGX(in vec3 n, in vec3 v, in float k)
{
	float nDotv = max(dot(n, v), 0.0);
	return nDotv / (nDotv * (1-k) + k);
}

vec3 GetWorldPositionFromDepth(in float depth, in vec2 fragCoords, in vec2 resolution, in mat4 invProj, in mat4 invView)
{
	vec2 ndc = vec2(
					(fragCoords.x / resolution.x) * 2.0 - 1.0,
					(fragCoords.y / resolution.y) * 2.0 - 1.0
				   );
	ndc.y *= -1.0;
	const vec4 clipPos = vec4(ndc, depth, 1.0);

	vec4 viewPos = invProj * clipPos;
	viewPos /= viewPos.w;

	vec4 worldPos = invView * viewPos;
	return worldPos.xyz;
}