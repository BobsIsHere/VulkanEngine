#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 fragColor;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D diffuseSampler;

vec3 lightPosition = vec3(1.2, 1.0, 2.0);
vec3 lightColor = vec3(0.7, 0.7, 1.0);

float roughness = 0.1;
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
    if (length(fragNormal) == 0.0)
    {
        outColor = vec4(fragColor, 1.0);
        return;
    }

    vec3 lightDirection = normalize(vec3(0.0, -1.0, -1.0));
    vec3 viewDirection = normalize(-inPosition);
    
    // Roughness squared
    const float roughnessSquared = roughness * roughness;

    // Calculate half vector
    vec3 halfVector = normalize(lightDirection + viewDirection);

    // Calculate fresnel
    vec3 f0 = vec3(0.0);

    // Specular variables
    vec3 f = FresnelFunction_Schlick(halfVector, viewDirection, f0);
    float d = NormalDistribution_GGX(fragNormal, halfVector, roughnessSquared);
    float g = Geometry_Smith(fragNormal, viewDirection, lightDirection, roughnessSquared);

    // Calculate specular
    vec3 DFG = d * f * g;
    float denominator = 4 * dot(viewDirection, fragNormal) * dot(lightDirection, fragNormal);
    vec3 specular = DFG / max(denominator, 0.001);

    if (metalness <= 0.0)
    {
        specular += Lambert(vec3(1.0, 1.0, 1.0) - f, albedo);
	}

    // Calculate the diffuse term
    vec3 diffuse = vec3(1.0) - f;
    diffuse *= max(dot(fragNormal, lightDirection), 0.0);

    // Sample the texture using texture coordinates
    vec4 textureColor = texture(diffuseSampler, fragTexCoord);

    // Combine the terms and output the color
    vec3 result = (diffuse + specular) * lightColor * albedo;
    outColor = textureColor * vec4(result, 1.0);
}