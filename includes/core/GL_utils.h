
#pragma once

#include <core/globalVariables.h>
#include <tinydir.h>


void renderCube();
void renderQuad();
void renderSphere();

//从framebuffer中存储图片 包含png和hdr图片
void save_image(const char* filepath, int image_width, int image_height, int channels, const string& file_extension);
//加载LUT和textureMap预览图
unsigned int loadTexture(const char* path);
//加载hdr图
unsigned int loadTexturef(const char* path);
//按照mipmap形式加载hdr图
unsigned int loadTexturef_MipMap(const char* path);
//加载五类textureMaps: albedo, metallic, roughness, normal, ao
unsigned int loadTextureMaps(const char* path);
//加载skybox图
unsigned int loadCubemap(vector<string> faces);
//用来加载含有prefilter的mipmap图 
unsigned int loadCubemap_MipMap(vector<string> faces);
//在已有cubemap的情况下 将prefilter加载成多层mipmap的形式
unsigned int loadTexturef_PackMipMap(const char* path);
//获得一个文件夹下所有的特定file-extension的文件数量
unsigned int GetFilesQuantity(const char* path, const char* file_extension);
//从precompute资源文件夹下获取文件路径
vector<string> ModifyFilepath(string TextureMap,string scenenames);//scenenames sceneindex 再"resources/textures/precompute/下获取文件名称
//使用stb_image加载纹理，并返回纹理的id
unsigned int TextureFromFile(const char* path, const string& directory, bool gamma = false);
//拼接两个const char*
char* concatenate_chars(const char* char1, const char* char2);







