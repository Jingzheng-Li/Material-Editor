
//通过tiny_dir读取所有的hdr model textureMaps文件，并且保存在全局变量scenes models等方便imgui读取做动态切换


#pragma once

#include <core/GL_utils.h>

class LoadFiles
{
public:

	void GetFiles(string Dir, vector<string>& Files, const char* File_extension);
	void AddFilepath();

	bool albedo_change = true;
	bool metallic_change = true;
	bool roughness_change = true;
	bool normal_change = true;
	bool ao_change = true;


private:
	vector<string> sceneFiles;
	vector<string> modelFiles;
	vector<string> albedoFiles;
	vector<string> metallicFiles;
	vector<string> normalFiles;
	vector<string> roughnessFiles;
	vector<string> aoFiles;

	//所有需要加载文件的地址，不应该暴露出来
	string HDRDir = FileSystem::getPath("resources/textures/hdr/");
	string modelDir = FileSystem::getPath("resources/objects/modelFiles/");
	string albedoDir = FileSystem::getPath("resources/textures/TextureMapping/albedo/");
	string metallicDir = FileSystem::getPath("resources/textures/TextureMapping/metallic/");
	string normalDir = FileSystem::getPath("resources/textures/TextureMapping/normal/");
	string roughnessDir = FileSystem::getPath("resources/textures/TextureMapping/roughness/");
	string aoDir = FileSystem::getPath("resources/textures/TextureMapping/ao/");

	string filename;
};




