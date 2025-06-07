#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(set = 0, binding = 0, std140) uniform UniformBufferObject
{
    mat4 viewProj;
    mat4 invView;
    mat4 invProj;
    vec3 cameraPos;
    float _pad0; // Padding to align next vec2

    vec2 screenSize;
    vec2 _pad1; // Padding to align next uint

    uint numLights;
    uint _pad2;
    uint _pad3;
    uint _pad4;
} ubo;

layout(set = 0, binding = 1) uniform sampler globalSampler;

layout(set = 0, binding = 6) uniform texture2D gAlbedo;
layout(set = 0, binding = 7) uniform texture2D gNormal;
layout(set = 0, binding = 8) uniform texture2D gMetal;
layout(set = 0, binding = 9) uniform texture2D gDepth;

struct Light
{
    mat4 viewProj;

    // Direction for directional lights, position for point lights
    vec3 direction_position;
    // Light type: 0 = directional, 1 = point
    uint type;

    vec3 color;
    float intensity;

    // Index into shadow texture array
    uint shadowMapIndex;
    int castsShadows;
};

layout(set = 0, binding = 11) uniform texture2D ShadowMapBuffer[];

layout(set = 0, binding = 12) buffer readonly LightDataBuffer
{
    Light lights[];
};

layout(set = 0, binding = 13) uniform samplerShadow shadowMapSampler;

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

vec3 EvaluateBRDF(vec3 N, vec3 V, vec3 L, vec3 albedo, float metalness, float roughness, float ao, vec3 irradiance);

void main()
{
    const ivec2 pixelCoords = ivec2(gl_FragCoord.xy);

    const vec4 albedo = texture(sampler2D(gAlbedo, globalSampler), fragUV);
    const vec4 metal = texture(sampler2D(gMetal, globalSampler), fragUV);
      
    const vec2 packedNormalXY = texture(sampler2D(gNormal, globalSampler), fragUV).xy;
    const float depth = texelFetch(
        sampler2D(gDepth, globalSampler), 
        pixelCoords, 0).r;

    //if (depth == 1.0f)
    //{
    //    discard;
    //}

	const float ao = metal.r;
	const float metalness = metal.b;
	const float roughness = clamp(metal.g, 0.05, 1.0);
	const float nSignZ = metal.a * 2.0 - 1.0;

	// Reconstruct normal from packed norma
    // TODO normal xy is off
    const vec2 nXY = packedNormalXY * 2.0 - 1.0;
    const float nZ = nSignZ * sqrt(max(0.0, 1.0 - dot(nXY, nXY)));
    const vec3 normal = normalize(vec3(nXY, nZ));

	// Reconstruct world position from depth
    const vec3 worldPos = GetWorldPosFromDepth(depth);

	const vec3 viewDir = normalize(ubo.cameraPos - worldPos);

    vec3 lighting = vec3(0.0f);

    for (int i = 0; i < ubo.numLights; ++i)
    {
        Light l = lights[i];

        // DIRECTIONAL LIGHT
        if (l.type == 0u)
        {
            vec4 lightSpacePos = l.viewProj * vec4(worldPos, 1.0f);

           // outColor = vec4(vec3(lightSpacePos.xyz), 1.0f);
            //outColor = vec4(vec3(lightSpacePos.w), 1.0f);
            lightSpacePos /= lightSpacePos.w;

            vec3 shadowMapUV = vec3(lightSpacePos.xy * 0.5f + 0.5f, lightSpacePos.z);
            //shadowMapUV.y = 1.0f - shadowMapUV.y;

            float shadow = texture(sampler2DShadow(ShadowMapBuffer[nonuniformEXT(l.shadowMapIndex)], shadowMapSampler),
                                          shadowMapUV).r;

            vec3 L = -normalize(l.direction_position);
            vec3 irradiance = l.color * l.intensity;

            lighting += EvaluateBRDF(normal, viewDir, L,
                albedo.rgb, metalness, roughness,
                ao, irradiance) * shadow;

        }
        // POINT LIGHT
        else if (l.type == 1u)
        {
            float shadowFactor = 1.0f;

            vec3 lightVec = l.direction_position - worldPos;
            float dist = length(lightVec);
            vec3  L = lightVec / dist;

            float attenuation = 1.0 / (dist * dist + 0.0001);
            vec3  irradiance = l.color * l.intensity * attenuation;

            lighting += EvaluateBRDF(normal, viewDir, L,
                albedo.rgb, metalness, roughness,
                ao, irradiance);
        }
    }

    const vec3 ambient = vec3(0.03) * albedo.rgb;
    const vec3 color = lighting + ambient;

    //color = color / (color + vec3(1.0f));
	//color = pow(color, vec3(1.0f / 2.2f));
    outColor = vec4(color, 1.0);

    // Material debugging
    //outColor = vec4(vec3(ao), 1.0f);
    //outColor = vec4(vec3(metalness), 1.0f);
    //outColor = vec4(vec3(roughness), 1.0f);

    // Fresnel Debugging
    //outColor = vec4(F0, 1.0);
    //outColor = vec4(F, 1.0);
    //outColor = vec4(G, 0, 0, 1.0);
    //outColor = vec4(NDF, 0, 0, 1.0);
    //outColor = vec4(numerator, 1.0);

    //outColor = vec4(specular, 1.0);

	// LightDir Debugging
    //outColor = vec4(vec3(NdotL), 1.0);

    // WorldPos debugging
    //outColor = vec4(worldPos, 1.0);
    //outColor = vec4(worldPos * 0.1f, 1.0); // Scale it down to avoid clamping

    // Viewdir debugging
    //float facing = dot(normal, viewDir);
    //outColor = vec4(vec3(facing * 0.5 + 0.5), 1.0);

    // Normal debugging
    //outColor = vec4(vec3(normal.x, normal.x, normal.x), 1.0);
    //outColor = vec4(vec3(normal.y, normal.y, normal.y), 1.0);
    //outColor = vec4(vec3(normal.z, normal.z, normal.z), 1.0);
    //outColor = vec4(normal, 1.0);
}

vec3 GetWorldPosFromDepth(float depth)
{
    // Convert from frag coordinate system to NDC
    vec2 ndc = vec2(
        (fragUV.x * 2 - 1),
        (fragUV.y * 2 - 1));
    //ndc.y *= -1.f;

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

    const bool squareRoughness = true;
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

vec3 EvaluateBRDF(vec3 N, vec3 V, vec3 L, vec3 albedo, float metalness, float roughness, float ao, vec3 irradiance)
{
    vec3 H = normalize(V + L);

    vec3 F0 = mix(vec3(0.04), albedo, metalness);
    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    float  NDF = DistributionGGX(N, H, roughness);
    float  G = GeometrySmith(N, V, L, roughness);
    vec3   num = NDF * G * F;
    float  den = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3   spec = num / den;

    vec3 kS = F;
    vec3 kD = (1.0 - kS) * (1.0 - metalness);

    float NdotL = max(dot(N, L), 0.0);
    return ((kD * albedo / gPI) * ao + spec) * irradiance * NdotL;
}