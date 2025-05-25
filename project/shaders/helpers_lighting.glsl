#include "constants.glsl"

// -- Normal Distrbution Function --
float ThrowbridgeReitzGGX(in vec3 n, in vec3 h, in float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float nDoth = max(dot(n, h), 0.0);
	float nDoth2 = nDoth * nDoth;

	float denom = (nDoth2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;
	return a2 / denom;
}

// -- Fresnel Schlick Function --
vec3 FresnelSchlick(in vec3 h, in vec3 v, in vec3 F0)
{
	float hDotv = max(dot(h, v), 0.0);
	return F0 + (1 - F0) * pow(1 - hDotv, 5);
}
vec3 FresnelSchlickRoughness(in vec3 h, in vec3 v, in vec3 F0, in float roughness)
{
	float hDotv = max(dot(h, v), 0.0);
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - hDotv, 5.0);
}

// -- Geometry Distribution Function --
float GeometrySchlickGGX_Direct(in vec3 n, in vec3 v, in float roughness)
{
	float r = 1.0 + roughness;
	float k = (r * r) / 8.0;

	float nDotv = max(dot(n, v), 0.0);
	return nDotv / (nDotv * (1-k) + k);
}
float GeometrySchlickGGX_Indirect(in vec3 n, in vec3 v, in float roughness)
{
	float k = (roughness * roughness) / 2.0;

	float nDotv = max(dot(n, v), 0.0);
	return nDotv / (nDotv * (1-k) + k);
}
float GeometrySmith(in vec3 n, in vec3 v, in vec3 l, in float roughness, in bool indirect)
{
	if(indirect)
		return GeometrySchlickGGX_Indirect(n, v, roughness) * GeometrySchlickGGX_Indirect(n, l, roughness);
	return GeometrySchlickGGX_Direct(n, v, roughness) * GeometrySchlickGGX_Direct(n, l, roughness);
}
