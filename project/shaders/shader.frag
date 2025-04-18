#version 450

// -- Texture Array Size --
layout(constant_id = 0) const uint TEXTURE_ARRAY_SIZE = 1;

// -- Texture Array Index --
layout(push_constant) uniform constants
{
	uint diffuseIdx;
	uint opacityIdx;
	uint specularIdx;
	uint shininessIdx;
	uint heightIdx;
	float expo;
} pushConstants;

// -- Texture Array --
layout(set = 1, binding = 0) uniform sampler2D textures[TEXTURE_ARRAY_SIZE];
layout(set = 2, binding = 0) uniform sampler2D shadowMap;

// -- Input --
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragTangent;
layout(location = 3) in vec3 fragBitangent;
layout(location = 4) in vec2 fragTexCoord;
layout(location = 5) in vec3 fragViewDir;

// -- Output --
layout(location = 0) out vec4 outColor;

// -- Shader --
void main()
{
	// -- Ugly Magenta --
	outColor = vec4(1.0, 0.0, 1.0, 1.0);
	
	// -- Diffuse --
	if(pushConstants.diffuseIdx < TEXTURE_ARRAY_SIZE)
		outColor.rgb = fragColor * texture(textures[pushConstants.diffuseIdx], fragTexCoord).rgb;

	// -- Normal --
	vec3 normal = normalize(fragNormal);
	vec3 tangent = normalize(fragTangent);
	vec3 bitangent = normalize(fragBitangent);
	if(pushConstants.heightIdx < TEXTURE_ARRAY_SIZE)
	{
		vec2 texelSize = 1.0 / textureSize(textures[pushConstants.heightIdx], 0);
		mat3x3 tbn = mat3x3(tangent, bitangent, normal);
		float scale = 1.0;

		float height = texture(textures[pushConstants.heightIdx], fragTexCoord).r;
		float deltaX = -scale * (height - texture(textures[pushConstants.heightIdx], fragTexCoord + vec2(texelSize.x, 0.0)).r);
		float deltaY = -scale * (height - texture(textures[pushConstants.heightIdx], fragTexCoord + vec2(0.0, texelSize.y)).r);
		vec3 bumpNormal = normalize(tbn * vec3(deltaX, deltaY, 1));
		normal = bumpNormal;
	}

	// -- Specular - Blinn-Phong  --
	vec3 lightDir = normalize(vec3(0.577, -0.577, 0.577));
	vec3 viewDir = normalize(fragViewDir);
	if(pushConstants.specularIdx < TEXTURE_ARRAY_SIZE)
	{
		vec3 ks = texture(textures[pushConstants.specularIdx], fragTexCoord).rgb;
		vec3 h = -normalize(viewDir + lightDir);
		float cosa = max(dot(h, normal), 0);
		outColor.rgb += ks * pow(cosa, pushConstants.expo);
	}

	// -- Lambert Cosine Law --
	//outColor.rgb *= max(dot(-lightDir, normal), 0);
	outColor.rgb *= (dot(-lightDir, normal) + 1) * 0.5;

	// -- Opacity --
	if(pushConstants.opacityIdx < TEXTURE_ARRAY_SIZE)
		outColor.a = texture(textures[pushConstants.opacityIdx], fragTexCoord).r;
}
