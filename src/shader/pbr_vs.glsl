
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

    //rotate��������Ϊrotate�ı���model��ͬʱ���ı���model�������model���󲻱䣬�������ܶ����Ǵ����
    WorldPos = vec3(model * vec4(aPos, 1.0));
       
    //�������ģ�Ͳ��ȱ�������ʱ���߲������ƫ��
    Normal = normalize(mat3(transpose(inverse(model))) * aNormal);
    Tangent = normalize(mat3(transpose(inverse(model))) * aTangent);
    Bitangent = normalize(mat3(transpose(inverse(model))) * aBitangent);

    //��worldPosͨ��lightMatrixת����lightSpace���� ����view�ӽ��ǲ������ı��
    WorldPosLightSpace = lightSpaceMatrix * vec4(WorldPos, 1.0);

    //view���ӽ���gl_Position������
    gl_Position =  projection * view * vec4(WorldPos, 1.0);

}