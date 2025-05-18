#version 450 core
#extension GL_GOOGLE_include_directive : require

// -- Includes --
#include "helpers.glsl"

// -- Camera --
layout(set = 0, binding = 0) uniform CameraUbo
{
	mat4 view;
	mat4 proj;
} cam;

// -- Lights --
struct Light
{
    vec4 dirpostype;
    vec3 color;
    float intensity;
};
layout(std430, set = 1, binding = 0) readonly buffer LightBuffer
{
	uint lightCount;
    Light lights[];
} lights;

// -- GBuffer --
layout(set = 2, binding = 0) uniform sampler2D Albedo_Opacity;
layout(set = 2, binding = 1) uniform sampler2D Normal;
layout(set = 2, binding = 2) uniform sampler2D WorldPos;
layout(set = 2, binding = 3) uniform sampler2D Roughness_Metallic;

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
	vec3 worldPos = texture(WorldPos, fragTexCoord).rgb;
	float roughness = clamp(texture(Roughness_Metallic, fragTexCoord).r, 0.001, 1.0);
	bool metal = texture(Roughness_Metallic, fragTexCoord).g > 0.5 ? true : false;

	vec3 n = normalize(texture(Normal, fragTexCoord).rgb * 2.0 - 1.0);
	vec3 v = -normalize(worldPos - inverse(cam.view)[3].xyz);
	float k = pow(roughness + 1, 2) / 8.0;
	vec3 F0 = metal ? albedo : vec3(0.04, 0.04, 0.04);

	// -- Ugly Magenta --
	outColor = vec4(1.0, 0.0, 1.0, 1.0);
	
	vec3 Lo = vec3(0);
	for(int lightIdx = 0; lightIdx < lights.lightCount; ++lightIdx)
	{
		// -- Extract Light Type --
		Light light = lights.lights[lightIdx];
		int type = int(round(light.dirpostype.w));
		vec3 l = vec3(0);
		vec3 radiance = vec3(0);

		// 0 == Directional Light
		if(type == 0)
		{
			l = -normalize(light.dirpostype.xyz);
			radiance = light.color;
		}

		// 1 == Point Light
		else if(type == 1)
		{
			l = normalize(light.dirpostype.xyz - worldPos);
			float dst = length(light.dirpostype.xyz - worldPos);
			float attenuation = 1.0 / (dst * dst);
			radiance = attenuation * light.color;
		}
		vec3 h = normalize(v + l);


		// -- Cook Torrence Specular BRDF --
		float D = ThrowbridgeReitzGGX(roughness * roughness, n, h);
		vec3 F = FresnelSchlick(h, l, F0);
		float G = GeometrySchlickGGX(n, v, k) * GeometrySchlickGGX(n, l, k);
		vec3 num = D * F * G;
		float denom = 4.0 * max(dot(n, v), 0.0) * max(dot(n, l), 0.0) + 0.0001;
		vec3 spec = num / denom;

		// -- Lambertian Diffuse BRDF --
		vec3 kd = metal ? vec3(0.0) : (vec3(1.0) - F);
		vec3 diff = kd * albedo / PI;

		// -- Lambert Cosine Law -- Observed Area --
		float oa = max(dot(l, n), 0);

		// -- Add to outgoing light --
		Lo += (diff + spec) * radiance * oa;
	}

	vec3 ambient = vec3(0.03) * albedo;
	vec3 color = ambient + Lo;
	outColor = vec4(color, alpha);
}
