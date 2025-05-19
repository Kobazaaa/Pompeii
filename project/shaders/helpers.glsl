// -- Constants --
float PI = 3.1415926535897932384626433832795028841971693993751058209;


// -- Normal Distrbution Function --
float ThrowbridgeReitzGGX(in float a, in vec3 n, in vec3 h)
{
	float a2 = a * a;
	float nDoth = max(dot(n, h), 0.0);
	return a2 / (PI * pow((nDoth * nDoth) * (a2 -1) + 1, 2));
}

// -- Fresnel Schlick Function --
vec3 FresnelSchlick(in vec3 h, in vec3 v, in vec3 F0)
{
	float hDotv = max(dot(h, v), 0.0);
	return F0 + (1 - F0) * pow(1 - hDotv, 5);
}

// -- Geometry Distribution Function --
float GeometrySchlickGGX(in vec3 n, in vec3 v, in float k)
{
	float nDotv = max(dot(n, v), 0.0);
	return nDotv / (nDotv * (1-k) + k);
}

// -- World Position --
vec3 GetWorldPositionFromDepth(in float depth, in vec2 fragCoords, in vec2 resolution, in mat4 invProj, in mat4 invView)
{
	vec2 ndc = vec2(
					(fragCoords.x / resolution.x) * 2.0 - 1.0,
					(fragCoords.y / resolution.y) * 2.0 - 1.0
				   );
	ndc.y *= -1.0;
	const vec4 clipPos = vec4(ndc, depth, 1.0);

	vec4 viewPos = invProj * clipPos;
	viewPos /= viewPos.w;

	vec4 worldPos = invView * viewPos;
	return worldPos.xyz;
}

// -- Tone Mappers --
vec3 ACESFilmToneMapping(in vec3 color)
{
	const float a = 2.51;
	const float b = 0.03;
	const float c = 2.43;
	const float d = 0.59;
	const float e = 0.14;
	return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
}
vec3 ReinhardToneMapping(in vec3 color)
{
	return color / (color + vec3(1.0));
}
vec3 Uncharted2ToneMappingCurve(in vec3 color)
{
	const float a = 0.15;
	const float b = 0.50;
	const float c = 0.10;
	const float d = 0.20;
	const float e = 0.02;
	const float f = 0.30;
	return ((color * (a * color + c * b) + d * e) / (color * (a * color + b) + d * f)) - e / f;
}
vec3 Uncharted2ToneMapping(in vec3 color)
{
	const float W = 11.2;
	const vec3 curvedColor = Uncharted2ToneMappingCurve(color);
	float whiteScale = 1.0 / Uncharted2ToneMappingCurve(vec3(W)).r;
	return clamp(curvedColor * whiteScale, 0.0, 1.0);
}

// -- Camera Exposure --
float CalculateEV100(in float aperture, in float shutterspeed, in float ISO)
{
	// EV100 = log2(N * N / t * 100 / S)
	// N: relative aperture (in f-stops)
	// t: shutter speed (in seconds)
	// S: sensor sensitivity (in ISO)

	// Sunny 16 rule (simulate a sunny day, using ISO 100)
	// N = 16, ISO = 100, t = 1 / ISO
	return log2(pow(aperture, 2) / shutterspeed * 100 / ISO);
}
float EV100ToExposure(in float EV100)
{
	const float maxLuminance = 1.2 * pow(2.0, EV100);
	return 1.0 / max(maxLuminance, 0.0001);
}