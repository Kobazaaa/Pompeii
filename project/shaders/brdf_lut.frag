#version 450 core
#extension GL_GOOGLE_include_directive : require

// -- Includes --
#include "helpers_lighting.glsl"

// -- Input --
layout(location = 0) in vec2 fragTexCoord;

// -- Output --
layout(location = 0) out vec2 outColor;

// -- Shader --
void main()
{
	// in LUT horizontal == nDotv && vertical == roughness
	const float NdotV = fragTexCoord.x;
	const float roughness = 1.0 - fragTexCoord.y;

	// integrate brdf
    vec3 v;
    v.x = sqrt(1.0 - NdotV * NdotV);
    v.y = 0.0;
    v.z = NdotV;

    float A = 0.0;
    float B = 0.0;
    vec3 n = vec3(0.0, 0.0, 1.0);

    uint sampleCount = 1024;
    for(uint i = 0; i < sampleCount; ++i)
    {
        // Generate a sampel vector that is biased towards the preferred
        // alignment direction
        vec2 Xi = Hammersley(i, sampleCount);
        vec3 h  = ImportanceSampleGGX(Xi, n, roughness);
        vec3 l  = normalize(2.0 * dot(v, h) * h - v);

        float NdotL = max(l.z, 0.0);
        float NdotH = max(h.z, 0.0);
        float VdotH = max(dot(v, h), 0.0);

        if(NdotL > 0.0)
        {
            float G = GeometrySmith(n, v, l, roughness, true);
            float G_Vis = (G * VdotH) / (NdotH * NdotV);
            float Fc = pow(1.0 - VdotH, 5.0);

            A += (1.0 - Fc) * G_Vis;
            B += Fc * G_Vis;
        }
    }
    A /= float(sampleCount);
    B /= float(sampleCount);
    outColor = vec2(A, B);
}
