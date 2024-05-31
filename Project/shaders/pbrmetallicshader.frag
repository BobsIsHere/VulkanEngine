#version 450

//--------------------------------------------
//   Variables
//--------------------------------------------

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragTangent;
layout(location = 3) in vec3 outViewDirection;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D albedoSampler;
layout(binding = 2) uniform sampler2D normalSampler;
layout(binding = 3) uniform sampler2D roughnessSampler;
layout(binding = 4) uniform sampler2D metalnessSampler;

//--------------------------------------------
//   Cook-Torrence
//--------------------------------------------
vec3 FresnelFunction_Schlick(vec3 h, vec3 v, vec3 f0)
{
    const float schlick = 1.0 - dot(h, v);
    return clamp(f0 + (1.f - f0) * (schlick * schlick * schlick * schlick * schlick), 0.f, 1.f);
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

vec3 Lambert(vec3 kd, vec3 cd)
{
    const vec3 rho = (cd * kd);
    return rho / 3.14159265358979323846f;
}

//--------------------------------------------
//   Main Function
//--------------------------------------------

void main() 
{
    vec3 albedoTexture = texture(albedoSampler, fragTexCoord).rgb;
    vec3 normalTexture = texture(normalSampler, fragTexCoord).rgb;
    float roughnessTexture = texture(roughnessSampler, fragTexCoord).x;
    float metallicTexture = texture(metalnessSampler, fragTexCoord).x;

    // Calculate normals
    const vec3 binormal = cross(fragNormal, fragTangent);
    const mat3 tangentSpaceAxis = mat3(fragTangent, binormal, fragNormal);
    vec3 sampledNormal = 2.f * normalTexture - 1.f;
    sampledNormal = normalize(tangentSpaceAxis * sampledNormal);

    // Const Variables
    const vec3 lightDirection = normalize(vec3(0.577f, 0.577f, 0.577f));

    // Variables
    vec3 f0;
    vec3 H = normalize(outViewDirection + lightDirection);

    // Base reflectivity of the surface
    if (metallicTexture < 0.0001f) 
    { 
        f0 = vec3(0.04f, 0.04f, 0.04f); 
    }
    else 
    { 
        f0 = albedoTexture; 
    }

    // Calculate observed area
    const float observedArea = max(dot(sampledNormal, lightDirection), 0.f);

    vec3 F = FresnelFunction_Schlick(H, outViewDirection, f0);
    const float D = NormalDistribution_GGX(sampledNormal, H, roughnessTexture * roughnessTexture);
    float G = Geometry_Smith(sampledNormal, outViewDirection, lightDirection, roughnessTexture * roughnessTexture);

    vec3 numerator = F * D * G;
    vec3 denominator = vec3(4.f * dot(outViewDirection, sampledNormal) * dot(lightDirection, sampledNormal) + 0.001f);
    vec3 specular = numerator / denominator;

    vec3 diffuse = Lambert(1.f - F, albedoTexture);
    vec3 finalColor = (diffuse + specular) * observedArea;

    // Add a small ambient term to avoid complete blackness
    vec3 ambient = 0.1 * albedoTexture;
    finalColor += ambient;

    outColor = vec4(finalColor, 1.0);
}
