#version 450

// -- Data --
layout(set = 0, binding = 0) uniform sampler2D Albedo_Opacity;
layout(set = 0, binding = 1) uniform sampler2D Normal;
layout(set = 0, binding = 2) uniform sampler2D WorldPos;
layout(set = 0, binding = 3) uniform sampler2D Specularity;

//// -- Input --
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
	vec3 normal = normalize((texture(Normal, fragTexCoord).rgb - 0.5) * 2.0);
	
	// -- Specular - Blinn-Phong  --
	vec3 worldPos = texture(WorldPos, fragTexCoord).rgb;
	vec3 viewDir = normalize(vec3(0, 0, 0) - worldPos);
	float ks = texture(Specularity, fragTexCoord).r;
	vec3 h = -normalize(viewDir + vec3(0.577, -0.577, 0.577));
	float cosa = max(dot(h, normal), 0);
	outColor.rgb += ks * pow(cosa, 10);
	
	// -- Lambert Cosine Law --
	//outColor.rgb *= max(dot(-lightDir, normal), 0);
	outColor.rgb *= (dot(-vec3(0.577, -0.577, 0.577), normal) + 1) * 0.5;

	//vec3 projCoords = fragShadowPos.xyz / fragShadowPos.w;
	//projCoords.xy = projCoords.xy * 0.5 + 0.5;
    //float closestDepth = texture(shadowMap, projCoords.xy).r;
    //float currentDepth = projCoords.z;
	//if(currentDepth > closestDepth)
	//	outColor.rgb *= 0.2;
}
