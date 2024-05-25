#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 fragColor;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 fragNormal;
layout(location = 4) in vec3 fragTangent;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D diffuseSampler;
layout(binding = 2) uniform sampler2D normalSampler;
layout(binding = 3) uniform sampler2D roughnessSampler;

vec3 lightPosition = vec3(1.2, 1.0, 2.0);
vec3 lightColor = vec3(0.7, 0.7, 1.0);

float metalness = 0.0;
vec3 albedo = vec3(0.8, 0.6, 0.4);

vec3 Lambert(vec3 kd, vec3 cd)
{
	const vec3 rho = cd * kd;
    return rho / 3.14159265359;
}

vec3 FresnelFunction_Schlick(vec3 h, vec3 v, vec3 f0)
{
    const float schlick = 1.0 - dot(h, v);

	return f0 + (vec3(1.0, 1.0, 1.0) - f0) * (schlick * schlick * schlick * schlick * schlick);
}

float NormalDistribution_GGX(vec3 n, vec3 h, float roughness)
{
    const float a = roughness * roughness;
    const float dpSquared = dot(n, h) * dot(n, h);
    const float denominator = 3.14159265359 * ((dpSquared * (a - 1.0) + 1.0) * (dpSquared * (a - 1.0) + 1.0));

    return a / denominator;
}

float Geometry_SchlickGGX(vec3 n, vec3 v, float roughness)
{
    const float dp = dot(n, v);
    const float k = ((roughness + 1.0) * (roughness + 1.0)) / 8.0;

    return dp / (dp * (1.0 - k) + k);
}

float Geometry_Smith(vec3 n, vec3 v, vec3 l, float roughness)
{
    const float smith1 = Geometry_SchlickGGX(n, v, roughness);
    const float smith2 = Geometry_SchlickGGX(n, l, roughness);

    return smith1 * smith2;
}

void main() 
{
    vec3 lightDirection = normalize(vec3(0.0, -1.0, -1.0));
    vec3 viewDirection = normalize(-inPosition);

    vec3 diffuseTexture = texture(diffuseSampler, fragTexCoord).rgb;
    vec3 normalTexture = texture(normalSampler, fragTexCoord).rgb * 2.0 - 1.0;
    float roughnessTexture = texture(roughnessSampler, fragTexCoord).x;
    
    // Roughness squared
    const float roughnessSquared = roughnessTexture * roughnessTexture;

    // Calculate half vector
    vec3 halfVector = normalize(lightDirection + viewDirection);

    // Calculate fresnel
    vec3 f0 = diffuseTexture;

    // Specular variables
    vec3 f = FresnelFunction_Schlick(halfVector, viewDirection, f0);
    float d = NormalDistribution_GGX(normalTexture, halfVector, roughnessSquared);
    float g = Geometry_Smith(normalTexture, viewDirection, lightDirection, roughnessSquared);

    // Calculate specular
    vec3 DFG = d * f * g;
    float denominator = 4 * dot(viewDirection, normalTexture) * dot(lightDirection, normalTexture);
    vec3 specular = DFG / denominator;

    if (metalness <= 0.0)
    {
        specular += Lambert(vec3(1.0, 1.0, 1.0) - f, albedo);
	}

    // Combine the terms and output the color
    vec3 result = (diffuseTexture);
    outColor = vec4(result, 1.0);
}