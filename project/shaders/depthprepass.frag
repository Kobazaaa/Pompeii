#version 450
#extension GL_EXT_nonuniform_qualifier : require

// -- Texture Array Index --
layout(push_constant) uniform constants
{
	layout(offset = 64) 
	uint diffuseIdx;
	uint opacityIdx;
	uint textureCount;
} pushConstants;

// -- Data --
layout(set = 1, binding = 0) uniform sampler2D textures[];

// -- Input --
layout(location = 0) in vec2 fragTexCoord;

// -- Shader --
void main()
{
	float alpha = 1.0;

	// -- Diffuse --
	if(pushConstants.diffuseIdx < pushConstants.textureCount)
		alpha = texture(textures[nonuniformEXT(pushConstants.diffuseIdx)], fragTexCoord).a;
	// -- Opacity --
	if(pushConstants.opacityIdx < pushConstants.textureCount)
		alpha = texture(textures[nonuniformEXT(pushConstants.opacityIdx)], fragTexCoord).r;
	// -- Alpha Cutout --
	if(alpha < 0.95)
		discard;
}