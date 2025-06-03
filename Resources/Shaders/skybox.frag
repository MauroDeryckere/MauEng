#version 450

layout(location = 0) in vec3 inFragLocalPos;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform texture2D equirectangularMap;
layout(set = 0, binding = 1) uniform sampler hdriSampler;

const vec2 gInvATan = vec2(0.1591f, 0.3183f);
vec2 SampleSphericalMap(vec3 v)
{
	vec2 UV = vec2( atan(v.z, v.x), 
					asin(v.y));

	UV *= gInvATan;
	UV += 0.5f;
	return UV;
}

void main()
{
	// Get Pos and use as a dir to sample
	vec3 dir = normalize(inFragLocalPos);
	// May need to flip dirs here - TODO
	//dir = vec3()
	const vec2 UV = SampleSphericalMap(dir);
	outColor = vec4(texture(sampler2D(equirectangularMap, hdriSampler), UV).rgb, 1.0f);
}