#version 450
#extension GL_EXT_nonuniform_qualifier : require

// -- Texture Array Index --
layout(push_constant) uniform constants
{
	layout(offset = 64) 
	uint diffuseIdx;
	uint opacityIdx;
	uint normalIdx;
	uint roughnessIdx;
	uint metallicIdx;
	uint textureCount;
} pushConstants;

// -- Data --
layout(set = 1, binding = 0) uniform sampler2D textures[];

// -- Input --
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragTangent;
layout(location = 3) in vec3 fragBitangent;
layout(location = 4) in vec2 fragTexCoord;
layout(location = 5) in vec3 fragWorldPos;

// -- Output --
layout(location = 0) out vec4 outAlbedo_Opacity;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outWorldPos;
layout(location = 3) out vec2 outRoughness_Metallic;

// -- Shader --
void main()
{
	// -- Ugly Magenta --
	outAlbedo_Opacity = vec4(1.0, 0.0, 1.0, 1.0);
	
	// -- Diffuse --
	if(pushConstants.diffuseIdx < pushConstants.textureCount)
		outAlbedo_Opacity.rgb = fragColor * texture(textures[nonuniformEXT(pushConstants.diffuseIdx)], fragTexCoord).rgb;
	// -- Opacity --
	if(pushConstants.opacityIdx < pushConstants.textureCount)
		outAlbedo_Opacity.a = texture(textures[nonuniformEXT(pushConstants.opacityIdx)], fragTexCoord).r;

	// -- Normal --
	vec3 normal = normalize(fragNormal);
	vec3 tangent = normalize(fragTangent);
	vec3 bitangent = normalize(fragBitangent);
	if(pushConstants.normalIdx < pushConstants.textureCount)
	{
		mat3x3 tbn = mat3x3(tangent, bitangent, normal);
		vec3 sampledNormal = texture(textures[nonuniformEXT(pushConstants.normalIdx)], fragTexCoord).rgb * 2.0 - 1.0;
		normal = normalize(tbn * sampledNormal);
	}
	outNormal = vec4(normal * 0.5 + 0.5, 1.0);

	// -- Specular --
	outRoughness_Metallic.r = 0.0;
	outRoughness_Metallic.g = 0.0;
	if(pushConstants.roughnessIdx < pushConstants.textureCount)
		outRoughness_Metallic.r = texture(textures[nonuniformEXT(pushConstants.roughnessIdx)], fragTexCoord).g;
	if(pushConstants.metallicIdx < pushConstants.textureCount)
		outRoughness_Metallic.g = texture(textures[nonuniformEXT(pushConstants.metallicIdx)], fragTexCoord).b;
	
	// -- World Pos --
	outWorldPos = vec4(fragWorldPos, 1.0);
}
