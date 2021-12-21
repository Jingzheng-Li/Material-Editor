
#pragma once

#include <core/globalVariables.h>
#include <tinydir.h>


void renderCube();
void renderQuad();
void renderSphere();

//��framebuffer�д洢ͼƬ ����png��hdrͼƬ
void save_image(const char* filepath, int image_width, int image_height, int channels, const string& file_extension);
//����LUT��textureMapԤ��ͼ
unsigned int loadTexture(const char* path);
//����hdrͼ
unsigned int loadTexturef(const char* path);
//����mipmap��ʽ����hdrͼ
unsigned int loadTexturef_MipMap(const char* path);
//��������textureMaps: albedo, metallic, roughness, normal, ao
unsigned int loadTextureMaps(const char* path);
//����skyboxͼ
unsigned int loadCubemap(vector<string> faces);
//�������غ���prefilter��mipmapͼ 
unsigned int loadCubemap_MipMap(vector<string> faces);
//������cubemap������� ��prefilter���سɶ��mipmap����ʽ
unsigned int loadTexturef_PackMipMap(const char* path);
//���һ���ļ��������е��ض�file-extension���ļ�����
unsigned int GetFilesQuantity(const char* path, const char* file_extension);
//��precompute��Դ�ļ����»�ȡ�ļ�·��
vector<string> ModifyFilepath(string TextureMap,string scenenames);//scenenames sceneindex ��"resources/textures/precompute/�»�ȡ�ļ�����
//ʹ��stb_image�������������������id
unsigned int TextureFromFile(const char* path, const string& directory, bool gamma = false);
//ƴ������const char*
char* concatenate_chars(const char* char1, const char* char2);







