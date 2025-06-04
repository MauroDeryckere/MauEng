#version 450

layout(location = 0) in vec2 inFragUV;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler globalSampler;
layout(set = 0, binding = 10) uniform texture2D hdriImage;

layout(set = 0, binding = 14, std140) uniform CamSettingsUBO
{
    float aperture;
    float ISO;
    float shutterSpeed;
    float exposureOverride;

    uint mapper;
    uint isAutoExposure;
    uint enableExposure;

}camSettingsUBO;

vec3 ReinhardToneMap(vec3 x) 
{
    return x / (x + vec3(1.0));
}

vec3 Uncharted2ToneMap(vec3 x) 
{
    float A = 0.15;
    float B = 0.50;
    float C = 0.10;
    float D = 0.20;
    float E = 0.02;
    float F = 0.30;
    float W = 11.2;
    x = ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
    float whiteScale = ((W * (A * W + C * B) + D * E) / (W * (A * W + B) + D * F)) - E / F;
    return clamp(x / whiteScale, 0.0, 1.0);
}


vec3 ACESFilm(vec3 x) 
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;

    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}


// Calculate the EV100 value from physical camera parameters
float CalculateEV100FromPhysicalCamera(float aperture, float shutterTime, float ISO)
{
	// EV100 = log2((N * N) / (t * 100 / S));
	// N = relative aperture (f-number)
	// t = shutter time in seconds
	// S = sensor sensitivity/gain (ISO)
    // For "Sunny 16" rule, which simulates a sunny day, using ISO 100, use
	// N = 16, ISO = 100, t = 1/ISO

    return log2((aperture * aperture) / (shutterTime * 100.0 / ISO));
}


// Converts EV100 to exposure value
float ConvertEV100ToExposure(float EV100)
{
	// Calculate the max luminance possible with H_sbs sensitivity as given by Lagarde
    const float maxLum = 1.2f * EV100 * EV100;
	return 1.0f / max(maxLum, 0.0001f);
}

float CalculateEV100FromAverageLuminance(float averageLuminance)
{
    // Following calculation assumes that a properly exposed image would place
    // a "middle gray" surface (reflecting 18.5% of light) at a luminance value
	// corresponding to EV100 = 0. when luminance = about 0.125 cd/(m^2).
    
    // Note the concept of "middle gray" is key in exposure compoensation
    // because camera systems ad tone mapping aim to map average screen brightness
    // around this neutral point for natural looking results
	// K is a calibration constant that adjusts the scale of the logarithmic mapping (typically 12.5f for reflected light meters)
    const float K = 12.5f;
	return log2(averageLuminance * 100.f) / K;
}

void main()
{
    float exposure = camSettingsUBO.exposureOverride;

    if (camSettingsUBO.enableExposure == 1 && exposure == 0.0f)
    {
        const float EV100 = 1.0f;
        const float EV100PhysicalCam = CalculateEV100FromPhysicalCamera(camSettingsUBO.aperture, camSettingsUBO.shutterSpeed, camSettingsUBO.ISO);
        exposure = ConvertEV100ToExposure(EV100PhysicalCam);
    }

    if (exposure == 0.0f)
    {
        exposure = 1.0f;
    }

    const vec3 hdrColor = texture(sampler2D(hdriImage, globalSampler), inFragUV).rgb;
    if (camSettingsUBO.mapper == 0)
    {
        const vec3 mapped = ACESFilm(hdrColor * exposure);
        outColor = vec4(mapped, 1.0);
    }
    if (camSettingsUBO.mapper == 1)
    {
        const vec3 mapped = Uncharted2ToneMap(hdrColor * exposure);
        outColor = vec4(mapped, 1.0);
    }
    if (camSettingsUBO.mapper == 2)
    {
        const vec3 mapped = ReinhardToneMap(hdrColor * exposure);
        outColor = vec4(mapped, 1.0);
    }
}