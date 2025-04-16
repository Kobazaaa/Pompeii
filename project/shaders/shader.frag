#version 450

// -- Texture Array Size --
layout(constant_id = 0) const uint TEXTURE_ARRAY_SIZE = 1;

// -- Texture Array Index --
layout(push_constant) uniform constants
{
	uint diffuseIdx;
	uint opacityIdx;
} pushConstants;

// -- Texture Array --
layout(set = 1, binding = 0) uniform sampler2D textures[TEXTURE_ARRAY_SIZE];

// -- Input --
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;

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

	// -- Opacity --
	if(pushConstants.opacityIdx < TEXTURE_ARRAY_SIZE)
		outColor.a = texture(textures[pushConstants.opacityIdx], fragTexCoord).r;
}