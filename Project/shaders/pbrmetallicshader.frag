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
    float schlick = 1.0 - dot(h, v);
    return clamp(f0 + (1.f - f0) * pow(schlick, 5.0), 0.f, 1.f);
}

float NormalDistribution_GGX(vec3 n, vec3 h, float roughness)
{
    float a = roughness * roughness;
    float dpSquared = dot(n, h) * dot(n, h);
    float denominator = 3.14159265359 * (dpSquared * (a - 1.0) + 1.0);
    denominator = denominator * denominator;
    return a / max(denominator, 1e-5); // Avoid division by zero
}

float Geometry_SchlickGGX(vec3 n, vec3 v, float roughness)
{
    float dp = dot(n, v);
    float k = (roughness * roughness) / 2.0;
    return dp / (dp * (1.0 - k) + k);
}

float Geometry_Smith(vec3 n, vec3 v, vec3 l, float roughness)
{
    float smith1 = Geometry_SchlickGGX(n, v, roughness);
    float smith2 = Geometry_SchlickGGX(n, l, roughness);
    return smith1 * smith2;
}

vec3 Lambert(vec3 kd, vec3 cd)
{
    vec3 rho = (cd * kd);
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
    vec3 binormal = cross(fragNormal, fragTangent);
    vec3 sampledNormal = normalize(2.f * normalTexture - 1.f);

    if (length(sampledNormal) < 1e-5) {
        outColor = vec4(1.0, 0.0, 0.0, 1.0); // Red for invalid normals
        return;
    }

    // Fixed light direction vector (same as the previous working code)
    vec3 lightDirection = normalize(vec3(0.577f, 0.577f, 0.577f));
    vec3 viewDirection = normalize(outViewDirection);

    // Check normal and view direction
    

    vec3 H = normalize(viewDirection + lightDirection);
    float observedArea = max(dot(sampledNormal, lightDirection), 0.f);

    // Base reflectivity of the surface
    vec3 f0 = mix(vec3(0.04), albedoTexture, metallicTexture);

    vec3 F = FresnelFunction_Schlick(H, viewDirection, f0);
    float D = NormalDistribution_GGX(sampledNormal, H, roughnessTexture);
    float G = Geometry_Smith(sampledNormal, viewDirection, lightDirection, roughnessTexture);

    vec3 numerator = F * D * G;
    float denominator = 4.0 * max(dot(viewDirection, sampledNormal), 0.0) * max(dot(lightDirection, sampledNormal), 0.0);
    denominator = max(denominator, 1e-5); // Avoid division by zero
    vec3 specular = numerator / denominator;

    vec3 kd = vec3(1.0) - F;
    kd *= 1.0 - metallicTexture;
    vec3 diffuse = Lambert(kd, albedoTexture);

    vec3 finalColor = (diffuse + specular) * observedArea;
    finalColor = max(finalColor, vec3(0.05, 0.05, 0.05)); // Minimum brightness threshold

    outColor = vec4(finalColor, 1.0);
}
