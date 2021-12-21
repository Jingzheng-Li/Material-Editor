

#version 330 core

#define ALBEDO_MAP 0
#define METALLIC_MAP 0
#define ROUGHNESS_MAP 0
#define NORMAL_MAP 0
#define AO_MAP 0

#define ANISOTROPY 1
#define CLEARCOAT 1
#define TRANSMISSION 1

//#define TEXTURE_WITH_ENVCUBEMAP
#define TEXTURE_WITH_HDR


out vec4 FragColor;

in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;
in vec3 Tangent;
in vec3 Bitangent;
in vec4 WorldPosLightSpace;

// material parameters
uniform vec3 albedo;
uniform float metallic;
uniform float roughness;
uniform float reflectance;
uniform float sheen;
uniform float sheenTint;
uniform float clearcoat;
uniform float clearcoatRoughness;
uniform float anisotropy;
uniform float env_brightness;


// IBL
uniform samplerCube irradianceMap;
uniform sampler2D brdfLUT;
uniform sampler2D bssrdfLUT;
uniform samplerCube prefilterMap;

//Sampling from HDR directly
uniform sampler2D equirectangularMap;
uniform sampler2D Irradiance_equirectangularMap;
uniform sampler2D Prefilter_equirectangularMap;

//texture map
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;

//subsurface
uniform sampler2D IBLLightPosition;
uniform float subsurfaceScale;
uniform float subsurfacePower;
uniform float thicknessScale;
uniform sampler2D depthMap;
uniform sampler2D frontdepthMap;

// lights
uniform vec3 PointlightPositions;
uniform vec3 PointlightColors;

uniform vec3 camPos;

const float PI = 3.14159265359;
const float PI_inverse = 0.31830988618;
const float MAX_REFLECTION_LOD = 8.0;
const float CC_MAX_REFLECTION_LOD = 10.0;


float saturate(float num) {return clamp(num, 0.0, 1.0);}
vec3 saturate(vec3 field) {return clamp(field, vec3(0.0), vec3(1.0));}


const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

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
// ----------------------------------------------------------------------------
float DistributionGGX(float NdotH, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH2 = NdotH*NdotH;
    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return nom / denom;
}


// ----------------------------------------------------------------------------
vec3 F_Schlick_roughness(float VdotH, vec3 F0, float roughness)
{
    vec3 Frough = max(vec3(1.0 - roughness), F0);
    float F = pow(saturate(1.0 - VdotH), 5.0);
    return Frough * F + F0 * (1.0 - F);
}
// ----------------------------------------------------------------------------
vec3 F_Schlick(float VdotH, vec3 F0)
{
    float F = pow(saturate(1.0 - VdotH), 5.0);
    return F + F0 * (1.0 - F);
}

float F_Schlick(float f0, float f90, float VdotH) 
{
    return f0 + (f90 - f0) * pow((1.0 - VdotH),5.0);
}

// Fresnel for complex IORs at specified angle
// F0(c)=(c-1)(c*-1)/(c+1)(c*+1)
float F_Schlick_IOR(float F0_real, float FO_complex, float cosTheta) 
{
    float cosTheta2 = cosTheta * cosTheta;
    float sinTheta2 = 1.0 - cosTheta2;

    float eta2 = F0_real * F0_real;
    float etak2 = FO_complex * FO_complex;

    float t0 = eta2 - etak2 - sinTheta2;
    float a2plusb2 = sqrt(t0 * t0 + 4 * eta2 * etak2);
    float t1 = a2plusb2 + cosTheta2;
    float a = sqrt(0.5f * (a2plusb2 + t0));
    float t2 = 2 * a * cosTheta;
    float Rs = (t1 - t2) / (t1 + t2);

    float t3 = cosTheta2 * a2plusb2 + sinTheta2 * sinTheta2;
    float t4 = t2 * sinTheta2;
    float Rp = Rs * (t3 - t4) / (t3 + t4);

    return 0.5f * (Rp + Rs);
}
// ----------------------------------------------------------------------------
float SchlickFresnelDiffuse(float cosTheta)
{
    float m = clamp(1-cosTheta, 0, 1);
    float m2 = m*m;
    return m2*m2*m; // pow(m,5)
}

float Fd_Burley(float roughness, float NoV, float NoL, float LoH) {
    // Burley 2012, "Physically-Based Shading at Disney"
    float f90 = 0.5 + 2.0 * roughness * LoH * LoH;
    float lightScatter = F_Schlick(1.0, f90, NoL);
    float viewScatter  = F_Schlick(1.0, f90, NoV);
    return lightScatter * viewScatter * (1.0 / PI);
}
vec3 prefilteredRadiance(const vec3 R, float roughness, float offset) 
{
    vec2 uv = SampleSphericalMap(normalize(R));
    float lod = 2.0 * roughness;
    return textureLod(Prefilter_equirectangularMap, uv, lod + offset).rgb;
}


// ----------------------------------------------------------------------------
vec3 GammaCorrect(vec3 x)
{
    return vec3(pow(x[0], 2.2), pow(x[1], 2.2), pow(x[2], 2.2));
}
// ----------------------------------------------------------------------------
vec4 ToneMap(in vec4 c, float limit)
{
    float luminance = 0.3*c.x + 0.6*c.y + 0.1*c.z;

    return c * 1.0 / (1.0 + luminance / limit);
}

// ----------------------------------------------------------------------------
vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(normalMap, TexCoords).xyz * 2.0 - 1.0;
    vec3 N = Normal;
    vec3 T = Tangent;
    vec3 B = Bitangent;
    mat3 TBN = mat3(T,B,N);
    return normalize(TBN * tangentNormal);
}
// ----------------------------------------------------------------------------

vec3 bentNormal(float anisotropy, vec3 N, vec3 V, vec3 T, vec3 B)
{
    vec3 anisotropicDirection = anisotropy >= 0.0 ? B : T; //bitanget的问题 在anis>0取bitang
    vec3 anisotropicTangent = cross(anisotropicDirection, V);
    vec3 anisotropicNormal = cross(anisotropicTangent, anisotropicDirection);
    vec3 bentNormal = normalize(mix(N, anisotropicNormal, anisotropy));
    return bentNormal;
}
// ----------------------------------------------------------------------------
vec3 ComputeFsheen(vec3 albedo, float NdotV)
{
    vec3 Cdlin = GammaCorrect(albedo);
    float Cdlum = 0.2126*Cdlin[0] + 0.7152*Cdlin[1]  + 0.0722*Cdlin[2]; // 求相对亮度
    vec3 Ctint = Cdlum > 0 ? Cdlin/Cdlum : vec3(1.0); //标准化albedo

    //sheen
    vec3 Csheen = mix(Ctint, vec3(1.0), sheenTint);
    float FH = SchlickFresnelDiffuse(NdotV);
    vec3 Fsheen = Csheen * FH * sheen;

    return Fsheen;
}
// ----------------------------------------------------------------------------
vec3 ComputediffuseColor(vec3 albedo, vec3 N, vec2 uv_normal)
{
    #ifdef TEXTURE_WITH_ENVCUBEMAP
    vec3 irradiance = texture(irradianceMap, N).rgb;
    #endif
    #ifdef TEXTURE_WITH_HDR
    vec3 irradiance = texture(Irradiance_equirectangularMap, uv_normal).rgb;
    #endif

    vec3 diffuseColor = albedo * irradiance;
    return diffuseColor;

}
// ----------------------------------------------------------------------------
vec3 ComputespecularColor(float NdotV, float roughness, vec3 R, vec2 uv, vec3 F_roughness)
{
    

    #ifdef TEXTURE_WITH_ENVCUBEMAP
    vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb; 
    #endif
    #ifdef TEXTURE_WITH_HDR
    vec3 prefilteredColor = textureLod(Prefilter_equirectangularMap, uv, roughness * MAX_REFLECTION_LOD).rgb; 
    #endif

    vec2 brdf  = texture(brdfLUT, vec2(NdotV, roughness)).rg;

    //vec3 specular = F_roughness * brdf.x + brdf.y;// origin brdf
    vec3 specular = mix(brdf.xxx, brdf.yyy, F_roughness);// brdf with multiscale
    vec3 specularColor = prefilteredColor * specular;

    return specularColor;
}
// ----------------------------------------------------------------------------
vec3 ComputeClearCoatColor(float clearcoatRoughness, vec3 R, vec2 uv)
{
    
    #ifdef TEXTURE_WITH_ENVCUBEMAP
    vec3 ClearCoatColor = textureLod(prefilterMap, R, clearcoatRoughness * CC_MAX_REFLECTION_LOD).rgb; 
    #endif
    #ifdef TEXTURE_WITH_HDR
    vec3 ClearCoatColor = textureLod(Prefilter_equirectangularMap, uv, clearcoatRoughness * CC_MAX_REFLECTION_LOD).rgb; 
    #endif

    return ClearCoatColor;
}



// ----------------------------------------------------------------------------
//change depth to linear
float near = 0.1; 
float far  = 7.5; 
float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return ((2.0 * near * far) / (far + near - z * (far - near))) / far;    
}
//calculate thickness base of PCF
float thicknessCalculation(vec4 worldposLightSpace, vec3 N, vec3 L, int meanscale)
{
    //这个thickness还是不行 在修改
    vec3 projCoords = worldposLightSpace.xyz / worldposLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(depthMap, projCoords.xy).r; 
    float currentDepth = projCoords.z;
    float bias = max(0.05 * (1.0 - dot(N, L)), 0.005);
    // PCF
    float thickness = 0.0;
    vec2 texelSize = 1.0 / textureSize(depthMap, 0);
    for(int x = - meanscale; x <= meanscale; ++x)
    {
        for(int y = - meanscale; y <= meanscale; ++y)
        {
            //在这里texture9次 取平均thickness
            float pcfDepth = texture(depthMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            thickness += currentDepth - bias - pcfDepth;        
        }    
    }
    thickness /= float(pow(2 * meanscale + 1, 2));
    if(projCoords.z > 1.0)
        thickness = 0.0;
    return thickness;
}
//PCF shadow
float shadowCalculation(vec4 worldposLightSpace, vec3 N, vec3 L)
{
    vec3 projCoords = worldposLightSpace.xyz / worldposLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(depthMap, projCoords.xy).r; 
    float currentDepth = projCoords.z;
    float bias = max(0.05 * (1.0 - dot(N, L)), 0.005);
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(depthMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(depthMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    if(projCoords.z > 1.0)
        shadow = 0.0;
    return shadow;
}
vec3 ComputeTransmissionColor(vec3 albedo, float thicknessScale, float subsurfaceScale, vec3 N, vec3 V)
{
    vec3 Light = vec3(0.0);
    vec3 L = normalize(PointlightPositions - WorldPos);
    vec3 H = normalize(L + N);
    float NdotH = saturate(dot(N,H));
    float NdotL = saturate(dot(N,L));
    float VdotH = saturate(dot(V,H));
    float thickness = (3.5 * thicknessScale + 0.5) * thicknessCalculation(WorldPosLightSpace, N, L, 2);
    float scatterVoH = saturate(dot(V,-L));
    float forwardScatter = exp2(scatterVoH * subsurfaceScale - subsurfaceScale);
    float backScatter = saturate(NdotL * thickness + (1.0 - thickness)) * 0.5;
    float subsurface = mix(backScatter, 1.0, forwardScatter) * (1.0 - thickness);
    vec3 transmissionColor = subsurface * albedo;
    return transmissionColor;
}


void main()
{		

    #if ALBEDO_MAP
    vec3 albedo_map = pow(texture(albedoMap, TexCoords).rgb, vec3(2.2));
    vec3 albedo = albedo * albedo_map;
    #endif

    #if METALLIC_MAP
    float metallic_map = texture(metallicMap, TexCoords).r;
    float metallic = metallic * metallic_map;
    #endif

    #if ROUGHNESS_MAP
    float roughness_map = texture(roughnessMap, TexCoords).r;
    float roughness = roughness * roughness_map;
    #endif

    #if AO_MAP
    float ao_map = texture(aoMap, TexCoords).r;
    float ao = ao_map;
    #endif

    #if NORMAL_MAP
    vec3 N = getNormalFromMap();
    vec3 T = Tangent;
    vec3 B = -normalize(cross(N, T));
    #else
    vec3 N = Normal;
    vec3 T = Tangent;
    vec3 B = Bitangent;
    #endif

    vec3 V = normalize(camPos - WorldPos);

    //set anisotropy normal
    #if ANISOTROPY
    N = bentNormal(anisotropy, N, V, T, B);
    #endif

    vec3 R = reflect(-V, N); 
    float NdotV = saturate(dot(N,V));

    //按照hdr采样时转换
    vec2 uv = SampleSphericalMap(normalize(R));
    vec2 uv_normal = SampleSphericalMap(normalize(N));

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)   
    // setup as relflectance later
    vec3 F0 = vec3(0.16 * reflectance * reflectance); 
    F0 = mix(F0, albedo, metallic);//mix F0*(1-metallic) + albedo * metallic
    vec3 F_roughness = F_Schlick_roughness(NdotV, F0, roughness); 
    vec3 F_clearcoatRoughness = F_Schlick_roughness(NdotV, vec3(0.04), clearcoatRoughness); 
    float F_fixed = F_Schlick(0.04, 1.0, NdotV);
    float Fcc = F_fixed * clearcoat;
    vec3 kS = F_roughness;
    vec3 kD = (1.0 - kS) * (1.0 - metallic);


    // environment lighting
    vec3 Fsheen = ComputeFsheen(albedo, NdotV);
    vec3 diffuseColor = ComputediffuseColor(albedo, N, uv_normal) + Fsheen;
    vec3 specularColor = ComputespecularColor(NdotV, roughness, R, uv, F_roughness);
    #if CLEARCOAT
    vec3 ClearCoatColor = ComputeClearCoatColor(clearcoatRoughness, R, uv);
    #endif
    #if TRANSMISSION
    //hack: tranmission加上clearcoat效果更好
    vec3 transmissionColor = ComputeTransmissionColor(albedo, thicknessScale, subsurfaceScale, N, V) + F_fixed * ComputeClearCoatColor(0.1, R, uv);
    diffuseColor = mix(diffuseColor, transmissionColor, subsurfacePower);
    #endif
     

    //TODO BSSRDF
    //=======================================================================================================
    //这本身是一个更难的过程 需要更多更复杂的考虑 主要用到的是normalized diffuse 需要有一个圆盘积分不知道怎么处理 r怎么来求解呢？
    //这个必须要想办法实现LUT 不然不可能实现在IBL中
    //基本的想法 把公式拆分成三个LUT来做
    //=======================================================================================================
  

    //gather to FragColor
    //=======================================================================================================

    #if CLEARCOAT
    //这个和kD有些关系 metallic大时 kD就会很小 导致diffuseColor没有办法照到
    vec3 environmentLight = kD * diffuseColor * (1.0 - Fcc) + specularColor * pow((1.0 - Fcc), 2.0) + ClearCoatColor * Fcc;
    #else
    vec3 environmentLight = kD * diffuseColor + specularColor;
    #endif

    //final_color = environment light + light source
    vec3 color = environmentLight * env_brightness;

    #if AO_MAP
    color = color * ao;   
    #endif

    // HDR tonemapping + gamma correction
    color = color / (color + vec3(1.0)); 
    color = pow(color, vec3(1.0/2.2)); 

    FragColor = vec4(color, 1.0);

     //=======================================================================================================

}


