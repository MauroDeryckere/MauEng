#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject
{
    mat4 viewProj;
    mat4 invView;
    mat4 invProj;
    vec3 cameraPos;
    vec2 screenSize;
} ubo;

layout(set = 0, binding = 1) uniform sampler globalSampler;

layout(set = 0, binding = 6) uniform texture2D gAlbedo;
layout(set = 0, binding = 7) uniform texture2D gNormal;
layout(set = 0, binding = 8) uniform texture2D gMetal;
layout(set = 0, binding = 9) uniform texture2D gDepth;

layout(location = 0) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

float gPI = 3.14159265359f;

vec3 GetWorldPosFromDepth(float depth);

// ratio between specular and diffuse reflection
// how much the surface reflects light versus how much it refracts light. 
vec3 FresnelSchlick(float cosTheta, vec3 F0);


float DistributionGGX(vec3 N, vec3 H, float roughness);

float GeometrySchlickGGX(float NdotV, float roughness);

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);

void main()
{
    vec4 albedo = texture(sampler2D(gAlbedo, globalSampler), fragUV);
    vec2 packedNormalXY = texture(sampler2D(gNormal, globalSampler), fragUV).xy;
    vec4 metal = texture(sampler2D(gMetal, globalSampler), fragUV);

    const float depth = texelFetch(
        sampler2D(gDepth, globalSampler), 
        ivec2(gl_FragCoord.xy), 0).r;

	const float ao = metal.r;
	const float metalness = metal.g;
	const float roughness = metal.b;

	// Reconstruct normal from packed normal
    vec2 nXY = packedNormalXY * 2.0 - 1.0;
    float nZ = sqrt(max(0.0, 1.0 - dot(nXY, nXY)));
    vec3 normal = normalize(vec3(nXY, nZ));

	// Reconstruct world position from depth
    const vec3 worldPos = GetWorldPosFromDepth(depth);

	const vec3 viewDir = normalize(ubo.cameraPos - worldPos);

    // Lighting
    const vec3 lightDir = -normalize(vec3(0.577f, -0.577f, -0.577f));
    const vec3 lightColor = vec3(1.0, 0.95, 0.9);
    const float intensity = 2.0f;

    //const float distance = length(lightPositions[i] - WorldPos);
    //const float attenuation = 1.0f / (distance * distance);
    //const vec3 radiance = lightColor * attenuation;
    
    // Only dir light for now
    const vec3 irradiance = lightColor * intensity;

    const vec3 H = normalize(viewDir + lightDir);

    const vec3 F0 = mix(vec3(0.04f), albedo.rgb, metalness);
    const vec3 F = FresnelSchlick(max(dot(H, viewDir), 0.0f), F0);

    // Cook-Torrance BRDF
    const float NDF = DistributionGGX(normal, H, roughness);
    const float G = GeometrySmith(normal, viewDir, lightDir, roughness);
    const vec3 numerator = NDF * G * F;
    // Note that we add 0.0001 to the denominator to prevent a divide by zero in case any dot product ends up 0.0.
    const float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0) + 0.0001;
    const vec3 specular = numerator / denominator;

    const vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metalness;

    float NdotL = max(dot(normal, lightDir), 0.0);
    vec3 lighting = (kD * albedo.rgb / gPI + specular) * irradiance * NdotL;
    const vec3 ambient = vec3(0.03) * albedo.rgb;

	vec3 color = lighting + ambient;
    //color = color / (color + vec3(1.0f));
	//color = pow(color, vec3(1.0f / 2.2f));

    outColor = vec4(color, 1.0);
}

vec3 GetWorldPosFromDepth(float depth)
{
    const ivec2 fragCoords = ivec2(gl_FragCoord.xy);

    // Convert from frag coordinate system to NDC
    vec2 ndc = vec2(
        (float(fragCoords.x) / ubo.screenSize.x) * 2.0f - 1.0f,
        (float(fragCoords.y) / ubo.screenSize.y) * 2.0f - 1.0f);

    ndc.y *= -1.f;

    const vec4 clipSpacePos = vec4(ndc, depth, 1.0f);

    // Inverse proj to view space
    vec4 viewSpacePos = ubo.invProj * clipSpacePos;
    viewSpacePos /= viewSpacePos.w;

    // Inverse view to world space
    const vec4 worldSpacePos = ubo.invView * viewSpacePos;

    return worldSpacePos.xyz;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness;

    const bool squareRoughness = false;
	if (squareRoughness)
		a = a * a;

    const float a2 = a * a;
    const float NdotH = max(dot(N, H), 0.0);
    const float NdotH2 = NdotH * NdotH;

    const float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = gPI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    const float r = (roughness + 1.0);
    const float k = (r * r) / 8.0;

    const float num = NdotV;
    const float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    const float NdotV = max(dot(N, V), 0.0);
    const float NdotL = max(dot(N, L), 0.0);
    const float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    const float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
