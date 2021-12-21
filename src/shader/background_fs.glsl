#version 330 core

//#define TEXTURE_WITH_ENVCUBEMAP
#define TEXTURE_WITH_HDR


out vec4 FragColor;
in vec3 WorldPos;

uniform samplerCube environmentMap;
uniform sampler2D equirectangularMap;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y)); //区间-PI到PI
    uv *= invAtan;
    uv += 0.5;//球面坐标转换到0到1
    return uv;
}

void main()
{	

    #ifdef TEXTURE_WITH_ENVCUBEMAP
    vec3 envColor = textureLod(environmentMap, WorldPos, 0.0).rgb;
    #endif

    #ifdef TEXTURE_WITH_HDR
    vec2 uv = SampleSphericalMap(normalize(WorldPos));
    vec3 envColor = textureLod(equirectangularMap, uv, 0.0).rgb;
    #endif
    
    // HDR tonemap and gamma correct
    envColor = envColor / (envColor + vec3(1.0));
    envColor = pow(envColor, vec3(1.0/2.2)); 
    
    FragColor = vec4(envColor, 1.0);

}
