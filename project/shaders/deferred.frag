#version 450

// -- Texture Array Size --
layout(constant_id = 0) const uint TEXTURE_ARRAY_SIZE = 1;

// -- Texture Array Index --
layout(push_constant) uniform constants
{
	layout(offset = 64) 
	uint diffuseIdx;
	uint opacityIdx;
	uint specularIdx;
	uint shininessIdx;
	uint heightIdx;
	float expo;
} pushConstants;

// -- Data --
layout(set = 1, binding = 0) uniform sampler2D textures[TEXTURE_ARRAY_SIZE];

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
layout(location = 3) out float outSpecularity;

// -- Shader --
void main()
{
	// -- Ugly Magenta --
	outAlbedo_Opacity = vec4(1.0, 0.0, 1.0, 1.0);
	
	// -- Diffuse --
	if(pushConstants.diffuseIdx < TEXTURE_ARRAY_SIZE)
		outAlbedo_Opacity.rgb = fragColor * texture(textures[pushConstants.diffuseIdx], fragTexCoord).rgb;
	// -- Opacity --
	if(pushConstants.opacityIdx < TEXTURE_ARRAY_SIZE)
		outAlbedo_Opacity.a = texture(textures[pushConstants.opacityIdx], fragTexCoord).r;

	// -- Normal --
	vec3 normal = normalize(fragNormal);
	vec3 tangent = normalize(fragTangent);
	vec3 bitangent = normalize(fragBitangent);
	outNormal = vec4(normal * 0.5 + 0.5, 1.0);
	if(pushConstants.heightIdx < TEXTURE_ARRAY_SIZE)
	{
		vec2 texelSize = 1.0 / textureSize(textures[pushConstants.heightIdx], 0);
		mat3x3 tbn = mat3x3(tangent, bitangent, normal);
		float scale = 1.0;

		float height = texture(textures[pushConstants.heightIdx], fragTexCoord).r;
		float deltaX = -scale * (height - texture(textures[pushConstants.heightIdx], fragTexCoord + vec2(texelSize.x, 0.0)).r);
		float deltaY = -scale * (height - texture(textures[pushConstants.heightIdx], fragTexCoord + vec2(0.0, texelSize.y)).r);
		vec3 bumpNormal = normalize(tbn * vec3(deltaX, deltaY, 1));
		outNormal = vec4(bumpNormal * 0.5 + 0.5, 1.0);
	}

	// -- Specular --
	outSpecularity.r = 0.0;
	if(pushConstants.specularIdx < TEXTURE_ARRAY_SIZE)
	{
		float ks = texture(textures[pushConstants.specularIdx], fragTexCoord).r;
		outSpecularity = ks;
	}

	// -- World Pos --
	outWorldPos.rgb = fragWorldPos.xyz;
}
