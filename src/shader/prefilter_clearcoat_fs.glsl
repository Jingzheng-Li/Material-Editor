#version 330 core
out vec4 FragColor;
in vec3 WorldPos;

uniform samplerCube environmentMap;
uniform float clearcoatRoughness;

const float PI = 3.14159265359;

// ----------------------------------------------------------------------------
// efficient VanDerCorpus calculation.
float RadicalInverse_VdC(uint bits) 
{
     bits = (bits << 16u) | (bits >> 16u);
     bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
     bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
     bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
     bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
     return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
// ----------------------------------------------------------------------------
vec2 Hammersley(uint i, uint N)
{
	return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}
// ----------------------------------------------------------------------------


float DistributionGTR1(vec3 N, vec3 H, float clearcoatRoughness)
{
    float NdotH = max(dot(N, H), 0.0);
    float a2 = clearcoatRoughness * clearcoatRoughness;
    float denom = 2*PI*log(clearcoatRoughness)*(a2 * cos(NdotH)*cos(NdotH) + sin(NdotH)*sin(NdotH));
    return (a2-1)/denom;
}
// ---------------------------------------------------------------------------

vec3 ImportanceSampleGTR1(vec2 Xi, vec3 N, float clearcoatRoughness)
//clearcoatGloss = mix(0.1,0.001)
{
    float clearcoatRoughness2 = clearcoatRoughness * clearcoatRoughness;

    float phi = 2.0 * PI * Xi.x;
    float cc_tmp = (pow(clearcoatRoughness, 2.0-2.0*Xi.y) -1.0)/(clearcoatRoughness2 -1.0);
    float cosTheta = sqrt(cc_tmp);
    float sinTheta = sqrt(1.0-(cosTheta * cosTheta));

    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;
    vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent   = normalize(cross(up, N));
	vec3 bitangent = cross(N, tangent);
	
	vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
	return normalize(sampleVec);
    
}
//------------------------------------------------------------------------
void main()
{		
    vec3 N = normalize(WorldPos);
    
    // make the simplyfying assumption that V equals R equals the normal 
    vec3 R = N;
    vec3 V = R;

    const uint SAMPLE_COUNT = 1024u;
    vec3 prefilteredColor = vec3(0.0);
    float totalWeight = 0.0;
    
    for(uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        // generates a sample vector that's biased towards the preferred alignment direction (importance sampling).
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        //vec3 H = ImportanceSampleGGX(Xi, N, roughness);
        vec3 H = ImportanceSampleGTR1(Xi, N, clearcoatRoughness);
        vec3 L  = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(dot(N, L), 0.0);
        if(NdotL > 0.0)
        {
            // sample from the environment's mip level based on roughness/pdf
            //float D   = DistributionGGX(N, H, roughness);
            float D = DistributionGTR1(N,H,clearcoatRoughness);
            float NdotH = max(dot(N, H), 0.0);
            float HdotV = max(dot(H, V), 0.0);
            float pdf = D * NdotH / (4.0 * HdotV) + 0.0001; 

            float resolution = 512.0; // resolution of source cubemap (per face)
            float saTexel  = 4.0 * PI / (6.0 * resolution * resolution);
            float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);

            float mipLevel = clearcoatRoughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel); 
            
            prefilteredColor += textureLod(environmentMap, L, mipLevel).rgb * NdotL;
            totalWeight      += NdotL;
        }
    }

    prefilteredColor = prefilteredColor / totalWeight;

    FragColor = vec4(prefilteredColor, 1.0);
}
