
#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;


out vec2 TexCoords;
out vec3 WorldPos;
out vec3 Normal;
out vec3 Tangent;
out vec3 Bitangent;
out vec4 WorldPosLightSpace;


uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat4 lightSpaceMatrix;

void main()
{
    TexCoords = aTexCoords;

    //rotate可行是因为rotate改变了model的同时，改变了model矩阵，如果model矩阵不变，整个可能都会是错误的
    WorldPos = vec3(model * vec4(aPos, 1.0));
       
    //这可以在模型不等比例缩放时法线不会产生偏移
    Normal = normalize(mat3(transpose(inverse(model))) * aNormal);
    Tangent = normalize(mat3(transpose(inverse(model))) * aTangent);
    Bitangent = normalize(mat3(transpose(inverse(model))) * aBitangent);

    //将worldPos通过lightMatrix转换到lightSpace下面 但是view视角是不发生改变的
    WorldPosLightSpace = lightSpaceMatrix * vec4(WorldPos, 1.0);

    //view的视角是gl_Position决定的
    gl_Position =  projection * view * vec4(WorldPos, 1.0);

}