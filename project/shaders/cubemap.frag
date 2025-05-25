#version 450 core

// -- Input --
layout(location = 0) in vec3 fragLocalPos;
layout(set = 0, binding = 0) uniform sampler2D hdri;

// -- Output --
layout(location = 0) out vec4 outColor;

// -- Functions --
const vec2 gInvAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(in vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= gInvAtan;
    uv += 0.5;
    return uv;
}

// -- Shader --
void main()
{
    vec3 dir = normalize(fragLocalPos);
    dir = vec3(dir.z, dir.y, dir.x);
    vec2 uv = SampleSphericalMap(dir);
    outColor = vec4(texture(hdri, uv).rgb, 1.0);
}
