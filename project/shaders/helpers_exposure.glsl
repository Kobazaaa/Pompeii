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
float AverageLuminanceToEV100(in float averageLuminance)
{
	const float K = 12.5;
	return log2((averageLuminance * 100.0) / K);
}

// -- Automatic Exposure --
#define RGB_TO_LUM vec3(0.2125, 0.7154, 0.0721)
uint ColorToBin(in vec3 hdrColor, in float minLogLum, in float inverseLogLumRange)
{
	// RGB to Lum
	float lum = dot(hdrColor, RGB_TO_LUM);

	// Avoid log of 0
	float epsilon = 0.001;
	if(lum < epsilon)
		return 0;

	// Calculate log2 luminance [0; 1], where 0 means minimum and 1 mean maximum luminance
	float logLum = clamp((log2(lum) - minLogLum) * inverseLogLumRange, 0.0, 1.0);

	// Map [0; 1] to [1, 255]. Bin 0 handled by epsilon check
	return uint(logLum * 254.0 + 1.0);
}