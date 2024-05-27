#version 450

//--------------------------------------------
//   Variables
//--------------------------------------------

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragTangent;

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

//--------------------------------------------
//   Main Function
//--------------------------------------------

void main() 
{
    // Const Variables
    const vec3 lightDirection = vec3(0.577f, 0.577f, 0.577f);
    const vec3 viewDirection = normalize(-inPosition);
    const vec3 albedo = vec3(0.972f, 0.960f, 0.915f);
    const float roughness = 1.f;
    const float metalness = 1.f;
    
    vec3 albedoTexture = texture(albedoSampler, fragTexCoord).rgb;
    vec3 normalTexture = texture(normalSampler, fragTexCoord).rgb;
    vec3 roughnessTexture = texture(roughnessSampler, fragTexCoord).rgb;
    vec3 metallicTexture = texture(metalnessSampler, fragTexCoord).rgb;

    // Variables
	const float roughnessSquared = roughness * roughness;
	const vec3 halfVector = normalize(viewDirection + lightDirection);
    vec3 f0;
    
    //base reflectivity of the surface
	if (metalness == 0.f) 
    { 
        f0 = vec3(0.04f, 0.04f, 0.04f); 
    }
	else 
    { 
        f0 = albedo; 
    }

    //specular variables
	const vec3 f = FresnelFunction_Schlick(halfVector, viewDirection, f0);
	const float d = NormalDistribution_GGX(fragNormal, halfVector, roughnessSquared);
	const float g = Geometry_Smith(fragNormal, viewDirection, lightDirection, roughnessSquared);

    //calculate specular
	const vec3 DFG = d * f * g;
	const float denominator = 4 * (dot(viewDirection, fragNormal) * dot(lightDirection, fragNormal));
	const vec3 specular = DFG / denominator;

    outColor = vec4(specular, 1.f);
}