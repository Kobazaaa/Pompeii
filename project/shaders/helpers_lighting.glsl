#ifndef HELPER_LIGHTING
#define HELPER_LIGHTING
#include "constants.glsl"
#include "helpers_general.glsl"

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

// -- Importance Sampling --
vec3 ImportanceSampleGGX(vec2 Xi, vec3 n, float roughness)
{
    // float a = roughness * roughness;
    float a = roughness;
	
    float phi = TWO_PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
	
    // from spherical coordinates to cartesian coordinates
    vec3 h;
    h.x = cos(phi) * sinTheta;
    h.y = sin(phi) * sinTheta;
    h.z = cosTheta;
	
    // from tangent-space vector to world-space sample vector
    vec3 tangent;
    vec3 bitangent;
	CalculateTangents(n, tangent, bitangent);
	
    vec3 sampleVec = tangent * h.x + bitangent * h.y + n * h.z;
    return normalize(sampleVec);
}
float RadicalInverse_VdC(in uint bits) 
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
vec2 Hammersley(in uint i, in uint N)
{
    return vec2(float(i) / float(N), RadicalInverse_VdC(i));
}

// -- Shadows --
float CalculateShadowTermDirectional(in mat4 lightSpace, in vec3 worldPos, in sampler2DShadow depth)
{
		vec4 lightSpacePos = lightSpace * vec4(worldPos, 1.0);
		lightSpacePos /= lightSpacePos.w;
		vec3 shadowMapUV = vec3(lightSpacePos.xy * 0.5 + 0.5, lightSpacePos.z);
		return texture(depth, shadowMapUV).r;
}
float CalculateShadowTermPoint(in vec3 lightPos, in vec3 worldPos, in samplerCubeShadow depth)
{
	return 1.0;
}

#endif //HELPER_LIGHTING
