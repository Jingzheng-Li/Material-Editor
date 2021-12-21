#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void main()
{
    //不应该啊 这里应该已经转移到lightspace下面了啊
    vec3 WorldPos = vec3(model * vec4(aPos, 1.0));   
    gl_Position = lightSpaceMatrix * vec4(WorldPos, 1.0);
}