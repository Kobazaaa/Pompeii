#version 450 core
#extension GL_GOOGLE_include_directive : require

// -- Includes --
#include "helpers.glsl"

// -- Data --
layout(set = 0, binding = 0) uniform sampler2D Albedo_Opacity;
layout(set = 0, binding = 1) uniform sampler2D Normal;
layout(set = 0, binding = 2) uniform sampler2D ViewDir;
layout(set = 0, binding = 3) uniform sampler2D Roughness_Metallic;
layout(set = 1, binding = 0) uniform LightUbo
{
	vec3 dir;
	vec3 color;
	float intensity;
} light;

// -- Input --
layout(location = 0) in vec2 fragTexCoord;

// -- Output --
layout(location = 0) out vec4 outColor;

// -- Shader --
void main()
{
	// -- Common Data --
	vec3 albedo = texture(Albedo_Opacity, fragTexCoord).rgb;
	float alpha = texture(Albedo_Opacity, fragTexCoord).a;
	vec3 n = normalize(texture(Normal, fragTexCoord).rgb * 2.0 - 1.0);
	vec3 v = -normalize(texture(ViewDir, fragTexCoord).rgb * 2.0 - 1.0);
	vec3 l = -normalize(light.dir);
	vec3 h = normalize(v + l);
	float roughness = clamp(texture(Roughness_Metallic, fragTexCoord).r, 0.001, 1.0);
	float k = pow(roughness + 1, 2) / 8.0;
	bool metal = texture(Roughness_Metallic, fragTexCoord).g > 0.5 ? true : false;

	// -- Ugly Magenta --
	outColor = vec4(1.0, 0.0, 1.0, 1.0);
	
	// -- Lambert Cosine Law --
	float oa = max(dot(l, n), 0);

	// -- Cook Torrence Specular --
	float D = ThrowbridgeReitzGGX(roughness * roughness, n, h);
	vec3 F = metal ? FresnelSchlick(h, l, albedo) : FresnelSchlick(h, l, vec3(0.04, 0.04, 0.04));
	float G = GeometrySchlickGGX(n, v, k) * GeometrySchlickGGX(n, l, k);
	float denom = 4.0 * dot(n, v) * dot(n, l);
	vec3 spec = (D * F * G) / denom;

	// -- Lambertian Diffuse --
	float kd = metal ? 0.0 : (1.0 - F.r);
	vec3 diff = kd * albedo / PI;

	// -- Output --
	outColor = vec4((diff + spec) * oa, alpha);
}
