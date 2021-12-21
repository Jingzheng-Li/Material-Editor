#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void main()
{
    //��Ӧ�ð� ����Ӧ���Ѿ�ת�Ƶ�lightspace�����˰�
    vec3 WorldPos = vec3(model * vec4(aPos, 1.0));   
    gl_Position = lightSpaceMatrix * vec4(WorldPos, 1.0);
}