#include "constants.glsl"

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
