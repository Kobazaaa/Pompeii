#version 450

// -- Data --
layout(set = 0, binding = 0) uniform sampler2D Albedo_Opacity;
layout(set = 0, binding = 1) uniform sampler2D Normal;
layout(set = 0, binding = 2) uniform sampler2D ViewDir;
layout(set = 0, binding = 3) uniform sampler2D Specularity;
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
	// -- Ugly Magenta --
	outColor = vec4(1.0, 0.0, 1.0, 1.0);
	
	// -- Diffuse + Opacity --
	outColor = texture(Albedo_Opacity, fragTexCoord);
	
	// -- Normal --
	vec3 normal = normalize(texture(Normal, fragTexCoord).rgb * 2.0 - 1.0);

	// -- Specular - Blinn-Phong  --
	vec3 viewDir = normalize(texture(ViewDir, fragTexCoord).rgb * 2.0 - 1.0);
	float ks = texture(Specularity, fragTexCoord).r;
	vec3 h = -normalize(viewDir + light.dir);
	float cosa = max(dot(h, normal), 0);
	outColor.rgb += ks * pow(cosa, 50);
	
	// -- Lambert Cosine Law --
	//outColor.rgb *= max(dot(-lightDir, normal), 0);
	outColor.rgb *= (dot(-light.dir, normal) + 1) * 0.5;
//	outColor.rgb = vec3(1,1,1) * (dot(-light.dir, normal) + 1) * 0.5;
//	outColor.rgb = normal;
}
