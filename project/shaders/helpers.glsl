// -- Constants --
float PI = 3.1415926535897932384626433832795028841971693993751058209;


// -- Normal Distrbution Function --
float ThrowbridgeReitzGGX(in float a, in vec3 n, in vec3 h)
{
	float a2 = a * a;
	float nDoth = dot(n, h);
	return a2 / (PI * pow((nDoth * nDoth) * (a2 -1) + 1, 2));
}

// -- Fresnel Schlick Function --
vec3 FresnelSchlick(in vec3 h, in vec3 v, in vec3 F0)
{
	float hDotv = dot(h, v);
	return F0 + (1 - F0) * pow(1 - hDotv, 5);
}

// -- Geometry Distribution Function --
float GeometrySchlickGGX(in vec3 n, in vec3 v, in float k)
{
	float nDotv = dot(n, v);
	return nDotv / (nDotv * (1-k) + k);
}
