

#version 330 core

//some Macro Definition
#define ALBEDO_MAP 1
#define ROUGHNESS_MAP 0
#define NORMAL_MAP 0
#define AO_MAP 0


out vec4 FragColor;

in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;
in vec3 Tangent;
in vec3 Bitangent;


uniform vec3 albedo;
uniform vec3 sheenColor;
uniform vec3 subsurfaceColor;
uniform float roughness;
uniform float anisotropy;
uniform float fabricscatter; //后面可修改
uniform float sheen; //后面可修改
uniform float sheenTint; //后面可修改
uniform float reflectance;
uniform float env_brightness;


//IBL
uniform samplerCube irradianceMap;
uniform sampler2D brdfLUT;
uniform samplerCube prefilterMap;


//Sampling from HDR directly
uniform sampler2D equirectangularMap;
uniform sampler2D Irradiance_equirectangularMap;
uniform sampler2D Prefilter_equirectangularMap;


//load TextureMaps
uniform sampler2D albedoMap;
uniform sampler2D roughnessMap;
uniform sampler2D normalMap;
uniform sampler2D aoMap;



const float PI = 3.14159265359;
const float PI_inverse = 0.31830988618;

uniform vec3 camPos;

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


float CharlieDistribution(float roughness, float NoH) {
    // Estevez and Kulla 2017, "Production Friendly Microfacet Sheen BRDF"
    float invAlpha  = 1.0 / roughness;
    float cos2h = NoH * NoH;
    float sin2h = max(1.0 - cos2h, 0.0078125); // 2^(-14/2), so sin2h^2 > 0 in fp16
    return (2.0 + invAlpha) * pow(sin2h, invAlpha * 0.5) / (2.0 * PI);
}

float NeubeltVisibility(float NoV, float NoL) {
    return 1.0 / (4.0 * (NoL + NoV - NoL * NoV) + 1e-5f);
}

vec3 F_Schlick_roughness(float VdotH, vec3 F0, float roughness)
{
    vec3 Frough = max(vec3(1.0 - roughness), F0);
    float F = pow(saturate(1.0 - VdotH), 5.0);
    return Frough * F + F0 * (1.0 - F);
}
vec3 F_Schlick(float VdotH, vec3 F0)
{
    float F = pow(saturate(1.0 - VdotH), 5.0);
    return F + F0 * (1.0 - F);
}

vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(normalMap, TexCoords).xyz * 2.0 - 1.0;
    vec3 N = Normal;
    vec3 T = Tangent;
    vec3 B = Bitangent;
    mat3 TBN = mat3(T,B,N);
    return normalize(TBN * tangentNormal);
}

vec3 GammaCorrect(vec3 x)
{
    return vec3(pow(x[0], 2.2), pow(x[1], 2.2), pow(x[2], 2.2));
}

//energy compensate 需要重新计算 把多重散射放到预计算积分中
//还有一个sheen color怎么能够放进去


void main()
{

    vec3 N = Normal;
    vec3 V = normalize(camPos - WorldPos);
    vec3 R = reflect(-V,N);
    vec3 F0 = vec3(reflectance);
    float NdotV = saturate(dot(N, V));
    vec2 uv = SampleSphericalMap(normalize(R));
    vec2 uv_normal = SampleSphericalMap(normalize(N));
    
    //vec3 kS = F_Schlick_roughness(NdotV, F0, roughness);
    vec3 kS = F_Schlick(NdotV, F0);
    vec3 kD = 1.0 - kS;

    #if ALBEDO_MAP
    vec3 albedo_map = pow(texture(albedoMap, TexCoords).rgb, vec3(2.2));
    vec3 albedo = albedo * albedo_map;
    #endif


    // diffuse
    //=============================================================================================================================
    vec3 Cdlin = GammaCorrect(albedo);
    float Cdlum = 0.2126*Cdlin[0] + 0.7152*Cdlin[1]  + 0.0722*Cdlin[2]; // 求相对亮度
    vec3 Ctint = Cdlum > 0 ? Cdlin/Cdlum : vec3(1.0); //标准化albedo

    //sheen
    vec3 Csheen = mix(Ctint, vec3(1.0), sheenTint);
    float FH = pow(saturate(1-NdotV), 5.0);
    vec3 Fsheen = Csheen * FH * sheen;

    vec3 irradiance = texture(Irradiance_equirectangularMap, uv_normal).rgb;
    vec3 diffuse = albedo * irradiance;
    float NoV = dot(N,V);
    vec3 indirectDiffuse = saturate((NoV + fabricscatter) / pow((1 + fabricscatter), 2)) * saturate(subsurfaceColor + NoV);
    vec3 diffuseColor = diffuse * indirectDiffuse + Fsheen * sheenColor;
    //=============================================================================================================================
    

    //specular //cloth时间上需要新的prefilterMap 先使用已有的prefilter即可
    //竟然都不是这些的问题 问题竟然出在一个shader影响到了另一个shader
    //=============================================================================================================================
    const float MAX_REFLECTION_LOD = 8.0;
    //vec3 prefilteredColor = textureLod(Prefilter_equirectangularMap, uv, roughness * MAX_REFLECTION_LOD).rgb; 
    vec3 prefilteredColor = textureLod(Irradiance_equirectangularMap, uv, roughness * MAX_REFLECTION_LOD).rgb; 
    float brdf  = texture(brdfLUT, vec2(NdotV, roughness)).b;
    vec3 specularColor = prefilteredColor * brdf;
    //=============================================================================================================================


    //=============================================================================================================================
    //这不对啊 这看起来完全不像布料 及其丑陋 为啥子布料这么失败
    vec3 environmentLight = kD * diffuseColor + specularColor;

    //final_color = environment light + light source
    vec3 color = environmentLight * env_brightness;

    // HDR tonemapping + gamma correction
    color = color / (color + vec3(1.0)); 
    color = pow(color, vec3(1.0/2.2)); 

    FragColor = vec4(color, 1.0);
    //=============================================================================================================================

}

