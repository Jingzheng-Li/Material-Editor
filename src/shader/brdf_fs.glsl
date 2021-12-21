#version 330 core

out vec3 FragColor;
in vec2 TexCoords;

//是不是这个saturate搞错了
float saturate(float num) {return clamp(num, 0.0, 1.0);}

const float PI = 3.14159265359;
// ----------------------------------------------------------------------------
//这里需要把现有的brdf加上multi-scatter

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
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
	float a = roughness*roughness;
	
	float phi = 2.0 * PI * Xi.x;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
	
	// from spherical coordinates to cartesian coordinates - halfway vector
	vec3 H;
	H.x = cos(phi) * sinTheta;
	H.y = sin(phi) * sinTheta;
	H.z = cosTheta;
	
	// from tangent-space H vector to world-space sample vector
	vec3 up          = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent   = normalize(cross(up, N));
	vec3 bitangent = cross(N, tangent);
	vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
	return normalize(sampleVec);
}
vec3 ImportanceSampleGGX_Filament(vec2 Xi, float roughness)
{
    float phi = 2.0 * PI * Xi.x;
    float cosTheta2 = (1.0 - Xi.y) / (1.0 + (roughness + 1.0) * ((roughness - 1.0) * Xi.y));
    float cosTheta = sqrt(cosTheta2);
    float sinTheta = sqrt(1.0 - cosTheta2);
    return vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    // note that we use a different k for IBL
    float a = roughness;
    float k = (a * a) / 2.0;
    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(float NdotV, float NdotL, float roughness)
{
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}
float GeometrySmith_Filament(float NdotL, float NdotV, float roughness)
{
    float a2 = roughness * roughness;
    float GGXL = NdotV * sqrt((-NdotL * a2 + NdotL) * NdotL + a2);
    float GGXV = NdotL * sqrt((-NdotV * a2 + NdotV) * NdotV + a2);
    return 0.5f / (GGXV + GGXL);
}

// ----------------------------------------------------------------------------
//compute in the pdf = DistributionCharlie() * cosTheta calculate cdf=P(sinTheta)=sinTheta^((2a+1)/a)=ksi => sinTheta = ksi^((a/2a+1))
vec3 ImportanceSampleCharlie(vec2 Xi, vec3 N, float roughness)
{
    float phi = 2.0 * PI * Xi.x;
    float sinTheta = pow(Xi.y, roughness/(2.0 * roughness + 1.0));
    float cosTheta = sqrt(1.0 - sinTheta * sinTheta);

    // from spherical coordinates to cartesian coordinates - halfway vector
	vec3 H;
	H.x = cos(phi) * sinTheta;
	H.y = sin(phi) * sinTheta;
	H.z = cosTheta;
	
	// from tangent-space H vector to world-space sample vector
	vec3 up          = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent   = normalize(cross(up, N));
	vec3 bitangent = cross(N, tangent);
	vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
	return normalize(sampleVec);
}

float GeometryAshikhmin(float NdotV, float NdotL)
{
    return 1.0 / (4.0 * (NdotL + NdotV - NdotL * NdotV));
}
float DistributionCharlie(float NdotH, float roughness)
{
    float invAlpha = 1.0 / roughness;
    float cos2h = NdotH * NdotH;
    float sin2h = 1 - cos2h;
    return (2.0 + invAlpha) * pow(sin2h, invAlpha * 0.5) / (2.0 * PI);
}

//----------------------------------------------------------------------------
vec2 IntegrateBRDF(float NdotV, float roughness)
{
    vec3 V;
    V.x = sqrt(1.0 - NdotV*NdotV);
    V.y = 0.0;
    V.z = NdotV;

    float A = 0.0;
    float B = 0.0; 

    vec3 N = vec3(0.0, 0.0, 1.0);
    
    const uint SAMPLE_COUNT = 1024u;
    for(uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        // generates a sample vector that's biased towards the
        // preferred alignment direction (importance sampling).
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        vec3 H = ImportanceSampleGGX(Xi, N, roughness);
        //vec3 H = ImportanceSampleGGX_Filament(Xi, roughness);
        vec3 L = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = saturate(L.z);
        float NdotH = saturate(H.z);
        float VdotH = saturate(dot(V, H));

        if(NdotL > 0.0)
        {
            float G = GeometrySmith(NdotV, NdotL, roughness); 
            //float G = GeometrySmith_Filament(NdotV, NdotL, roughness);
            float G_Vis = (G * VdotH) / (NdotH * NdotV);
            float Fc = pow(1.0 - VdotH, 5.0);
             A += G_Vis * (1.0 - Fc);
             B += G_Vis * Fc;
        }
    }
    A /= float(SAMPLE_COUNT);
    B /= float(SAMPLE_COUNT);
    return vec2(A, B);
}

vec2 IntegrateBRDF_Multiscatter(float NdotV, float roughness)
{
    vec3 V;
    V.x = sqrt(1.0 - NdotV*NdotV);
    V.y = 0.0;
    V.z = NdotV;

    float A = 0.0;
    float B = 0.0; 

    vec3 N = vec3(0.0, 0.0, 1.0);
    
    const uint SAMPLE_COUNT = 1024u;
    for(uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        // generates a sample vector that's biased towards the
        // preferred alignment direction (importance sampling).
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        vec3 H = ImportanceSampleGGX(Xi, N, roughness);
        vec3 L = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = saturate(L.z);
        float NdotH = saturate(H.z);
        float VdotH = saturate(dot(V, H));

        if(NdotL > 0.0)
        {
            float G = GeometrySmith(NdotV, NdotL, roughness);
            float G_Vis = (G * VdotH) / (NdotH * NdotV);
            float Fc = pow(1.0 - VdotH, 5.0);
             A += G_Vis * Fc;
             B += G_Vis;
        }
    }
    A /= float(SAMPLE_COUNT);
    B /= float(SAMPLE_COUNT);
    return vec2(A, B);
}

//cloth importance sampling take from 
float IntegrateClothBRDF(float NdotV, float roughness)
{
    vec3 V;
    V.x = sqrt(1.0 - NdotV*NdotV);
    V.y = 0.0;
    V.z = NdotV;
    float r = 0.0;
    vec3 N = vec3(0.0, 0.0, 1.0);

    const uint SAMPLE_COUNT = 1024u;
    for(uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        vec3 H = ImportanceSampleCharlie(Xi, N, roughness);
        vec3 L = normalize(2.0 * dot(V, H) * H - V);
        
        float NdotL = saturate(L.z);
        float NdotH = saturate(H.z);
        float VdotH = saturate(dot(V, H));

        if(NdotL > 0)
        {
            float J = 1.0f / (4.0f * VdotH);
            float pdf = NdotH;
            float G = GeometryAshikhmin(NdotV, NdotL);

            r += G * NdotL / (pdf * J);
        }

    }
    r /= float(SAMPLE_COUNT);
    return r;
}


// ----------------------------------------------------------------------------
void main() 
{
    //vec2 integratedBRDF = IntegrateBRDF(TexCoords.x, TexCoords.y);
    vec2 integratedBRDF = IntegrateBRDF_Multiscatter(TexCoords.x, TexCoords.y);
    float integratedClothBRDF = IntegrateClothBRDF(TexCoords.x, TexCoords.y);//需要考虑服装的roughness能量损失吗？
    FragColor = vec3(integratedBRDF, integratedClothBRDF);
}