
#include <core/LoadFiles.h>

//ͨ��tiny_dir��ȡ���е��ļ���
void LoadFiles::GetFiles(string Dir, vector<string>& Files, const char* File_extension)
{
	tinydir_dir dir;
	int i;
	tinydir_open_sorted(&dir, Dir.c_str());
	for (i = 0; i < dir.n_files; ++i)
	{
		tinydir_file file;
		tinydir_readfile_n(&dir, &file, i);
		if (string(file.extension) == File_extension) //should be like xxxxx.hdr
		{
			Files.push_back(Dir + string(file.name));
			//�������е�hdr����Ϊ�ļ�������
			if (Dir == HDRDir)
			{
				filename = string(file.name);
				scenenames.push_back(filename.substr(0, filename.rfind(".")));
			}
		}
	}
	tinydir_close(&dir);
}


void LoadFiles::AddFilepath()
{

	//��ѡ���ļ���չ�����ļ�����string��ʽ �����ڸ�������õ�Files��
	GetFiles(HDRDir, sceneFiles, "hdr");
	GetFiles(modelDir, modelFiles, "obj");
	GetFiles(modelDir, modelFiles, "fbx");
	GetFiles(albedoDir, albedoFiles, "png");
	GetFiles(metallicDir, metallicFiles, "png");
	GetFiles(normalDir, normalFiles, "png");
	GetFiles(roughnessDir, roughnessFiles, "png");
	GetFiles(aoDir, aoFiles, "png");

	//��Files��string�ļ������const char* ��ʽ������ȫ�ֱ����й�imgui��ȡ
	for (int i = 0; i < sceneFiles.size(); ++i) { scenes.push_back(sceneFiles[i].c_str()); }
	for (int i = 0; i < modelFiles.size(); ++i) { models.push_back(modelFiles[i].c_str()); }
	for (int i = 0; i < albedoFiles.size(); ++i) { albedoMaps.push_back(albedoFiles[i].c_str()); }
	for (int i = 0; i < metallicFiles.size(); ++i) { metallicMaps.push_back(metallicFiles[i].c_str()); }
	for (int i = 0; i < normalFiles.size(); ++i) { normalMaps.push_back(normalFiles[i].c_str()); }
	for (int i = 0; i < roughnessFiles.size(); ++i) { roughnessMaps.push_back(roughnessFiles[i].c_str()); }
	for (int i = 0; i < aoFiles.size(); ++i) { aoMaps.push_back(aoFiles[i].c_str()); }

}
