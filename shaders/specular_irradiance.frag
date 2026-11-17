#version 450 core
#extension GL_GOOGLE_include_directive : require

// -- Includes --
#include "helpers_lighting.glsl"

// -- Input --
layout(location = 0) in vec3 fragLocalPos;
layout(set = 0, binding = 0) uniform samplerCube envMap;

// -- Input PC --
layout(push_constant) uniform PushConstants {
	layout(offset = 128) 
    float roughness;
} pc;

// -- Output --
layout(location = 0) out vec4 outColor;

// -- Shader --
void main()
{
	// Sample direction is the hemisphere's orientation
	// make (simplifying) assumption that v == r == n
	vec3 n = normalize(fragLocalPos);
	n.y *= -1.0;
	vec3 r = n;
	vec3 v = n;

	// integrate over hemisphere w/ spherical coords
    uint sampleCount = 1024;
    float totalWeight = 0.0;
    vec3 prefilteredColor = vec3(0.0);     
    for(uint i = 0u; i < sampleCount; ++i)
    {
        // Generate a sampel vector that is biased towards the preferred
        // alignment direction
        vec2 Xi = Hammersley(i, sampleCount);
        vec3 h  = ImportanceSampleGGX(Xi, n, pc.roughness);
        vec3 l  = normalize(2.0 * dot(v, h) * h - v);

        float NdotL = max(dot(n, l), 0.0);
        if(NdotL > 0.0)
        {
            // mip chain of env/hdri map required
            float D = ThrowbridgeReitzGGX(n, h, pc.roughness);
            float NdotH = max(dot(n, h), 0.0);
            float HdotV = max(dot(h, v), 0.0);
            float pdf = D * NdotH / (4.0 * HdotV) + 0.0001;

            float resolution = textureSize(envMap, 0).x;
            float saTexel = 4.0 * PI / (6.0 * resolution * resolution);
            float saSample = 1.0 / (float(sampleCount) * pdf + 0.0001);

            float mipLevel = pc.roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);

            prefilteredColor += textureLod(envMap, l, mipLevel).rgb * NdotL;
            totalWeight      += NdotL;
        }
    }
    prefilteredColor = prefilteredColor / totalWeight;
    outColor = vec4(prefilteredColor, 1.0);
}
