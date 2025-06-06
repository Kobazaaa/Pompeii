#version 450 core
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require

// -- Includes --
#include "helpers_lighting.glsl"
#include "helpers_general.glsl"

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
    float luxLumen;
	uint matrixIndex;
	uint depthIndex;
};
layout(std430, set = 1, binding = 0) readonly buffer LightBuffer
{
	uint lightCount;
    Light lights[];
} lightBuffer;
layout(std430, set = 2, binding = 0) readonly buffer LightMatrices
{
	uint matrixCount;
    mat4 matrices[];
} lightSpaceMatrices;
layout(set = 3, binding = 0) uniform sampler2DShadow DirectionalShadowMaps[];
layout(set = 4, binding = 0) uniform samplerCubeShadow PointShadowMaps[];

// -- GBuffer & Surroundings --
layout(set = 5, binding = 0) uniform sampler2D Albedo_Opacity;
layout(set = 5, binding = 1) uniform sampler2D Normal;
layout(set = 5, binding = 2) uniform sampler2D WorldPos;
layout(set = 5, binding = 3) uniform sampler2D Roughness_Metallic;
layout(set = 5, binding = 4) uniform sampler2D Depth;
layout(set = 5, binding = 5) uniform samplerCube EnvironmentMap;
layout(set = 5, binding = 6) uniform samplerCube DiffuseIrradiance;
layout(set = 5, binding = 7) uniform samplerCube SpecularIrradiance;
layout(set = 5, binding = 8) uniform sampler2D BrdfLut;

// -- Input --
layout(location = 0) in vec2 fragTexCoord;

// -- Output --
layout(location = 0) out vec4 outColor;

// -- Shader --
void main()
{
	// -- Environment Map --
	float depth = texelFetch(Depth, ivec2(gl_FragCoord.xy), 0).r;
	if(depth >= 1.0)
	{
		const vec3 worldPos = GetWorldPositionFromDepth(
			1.0, ivec2(gl_FragCoord.xy), textureSize(Depth, 0),
			inverse(cam.proj), inverse(cam.view));
		vec3 sampleDir = normalize(worldPos);
		outColor = vec4(texture(EnvironmentMap, sampleDir).rgb, 1.0);
		return;
	}

	// -- Common Data --
	vec3 albedo = texture(Albedo_Opacity, fragTexCoord).rgb;
	float alpha = texture(Albedo_Opacity, fragTexCoord).a;
	vec3 worldPos = texture(WorldPos, fragTexCoord).rgb;
	float roughness = clamp(texture(Roughness_Metallic, fragTexCoord).r, 0.001, 1.0);
	roughness = roughness * roughness;
	float metalFactor = texture(Roughness_Metallic, fragTexCoord).g;
	bool metal = metalFactor > 0.5 ? true : false;

	vec3 n = normalize(texture(Normal, fragTexCoord).rgb * 2.0 - 1.0);
	vec3 v = -normalize(worldPos - inverse(cam.view)[3].xyz);
	vec3 F0 = metal ? albedo : vec3(0.04, 0.04, 0.04);

	// -- Ugly Magenta --
	outColor = vec4(1.0, 0.0, 1.0, 1.0);
	
	vec3 Lo = vec3(0);
	for(int lightIdx = 0; lightIdx < lightBuffer.lightCount; ++lightIdx)
	{
		// -- Extract Light Type --
		Light light = lightBuffer.lights[lightIdx];
		int type = int(round(light.dirpostype.w));
		vec3 l = vec3(0);
		vec3 radiance = vec3(0);

		// 0 == Directional Light
		if(type == 0)
		{
			l = -normalize(light.dirpostype.xyz);
			float illuminance = light.luxLumen;
			radiance = illuminance * light.color;
		}

		// 1 == Point Light
		else if(type == 1)
		{
			l = normalize(light.dirpostype.xyz - worldPos);

			float luminousIntensity = light.luxLumen / (4.0 * PI);
			float dst = length(light.dirpostype.xyz - worldPos);
			float attenuation = 1.0 / max((dst * dst), 0.0001);
			float illuminance = attenuation * luminousIntensity;
			radiance = illuminance * light.color;
		}
		vec3 h = normalize(v + l);


		// -- Cook Torrence Specular BRDF --
		float D = ThrowbridgeReitzGGX(n, h, roughness);
		vec3 F = FresnelSchlick(h, v, F0);
		float G = GeometrySmith(n, v, l, roughness, false);
		vec3 num = D * F * G;
		float denom = 4.0 * max(dot(n, v), 0.0001) * max(dot(n, l), 0.0001);
		vec3 spec = num / denom;
		// -- Lambertian Diffuse BRDF --
		vec3 kd = vec3(1.0) - F;
		kd *= 1.0 - metalFactor;
		vec3 diff = kd * albedo / PI;

		// -- Lambert Cosine Law -- Observed Area --
		float oa = max(dot(l, n), 0);

		// -- Shadow --
		float shadowTerm = 1.0;
		if(type == 0) // 0 == Directional Light
			shadowTerm = CalculateShadowTermDirectional(lightSpaceMatrices.matrices[light.matrixIndex], worldPos, DirectionalShadowMaps[light.depthIndex]);
		else if(type == 1) // 1 == Point Light
			shadowTerm = CalculateShadowTermPoint(light.dirpostype.xyz, worldPos, PointShadowMaps[light.depthIndex]);

		// -- Add to outgoing light --
		Lo += (diff + spec) * radiance * oa * shadowTerm;
	}

	vec3 F = FresnelSchlickRoughness(n, v, F0, roughness);
	vec3 kd = (1.0 - F) * (1.0 - metalFactor);
	vec3 diffuseIrradiance = texture(DiffuseIrradiance, vec3(n.x, -n.y, n.z)).rgb;
	vec3 diffuse = kd * diffuseIrradiance * albedo;

	const float maxLod = 4.0;
	vec3 refl = reflect(-v, n);
	const vec3 specularIrradiance = textureLod(SpecularIrradiance, refl, roughness * maxLod).rgb;
	const vec2 brdfLUT = texture(BrdfLut, vec2(max(dot(n, v), 0.0), roughness)).rg;
	vec3 specular = specularIrradiance * (F0 * brdfLUT.x + brdfLUT.y);
	// I'm unsure if in the above formula i should use F or F0. All example code uses F, but the mathematical formula uses F0.
	// I decided to use F0 as it gave me better results, but maybe I am missing something


	vec3 ambient = (diffuse + specular);
	vec3 color = ambient + Lo;
	outColor = vec4(color, alpha);
}
