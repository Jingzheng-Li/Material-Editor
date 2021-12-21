
//Ԥ����TOOLS���� ����
#pragma once

#include <core/GL_utils.h>


class CubemapPrecompute
{

public:
	CubemapPrecompute() {};
	~CubemapPrecompute() {};

public:
	//��hdrת��Ϊcubemap��precompute process 
	unsigned int equirectangular_to_cubemap(Shader ourshader, unsigned int hdrTexture);
	unsigned int PrecomputeIrradianceMap(Shader ourshader, unsigned int irradianceMaptest);
	unsigned int PrecomputePrefilterMap(Shader ourshader, unsigned int prefilterMap, unsigned int mip);
	void renderCube();
	unsigned int loadTexturef(const char* path);


protected:

	// ��hdrͼת����cubemapͼƬ
	Shader equirectangularToCubemapShader = { "cubemap_vs.glsl", "equirectangular_to_cubemap_fs.glsl" };
	Shader irradianceShader = { "cubemap_vs.glsl", "irradiance_convolution_fs.glsl" };
	Shader prefilterShader = { "cubemap_vs.glsl", "prefilter_fs.glsl" };


	//����brdf��LUT
	Shader brdfShader = { "brdf_vs.glsl", "brdf_fs.glsl" };


protected:

	unsigned int hdrTexture;
	unsigned int envCubemap;
	unsigned int irradianceMap;
	unsigned int prefilterMap0, prefilterMap1, prefilterMap2, prefilterMap3, prefilterMap4;
	


	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 captureViews[6] =
	{
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

};


class HDRPrecompute
{
public:

	//ֱ�Ӵ�hdr������precompute process
	unsigned int cubemap_to_equirectangular(unsigned int precomputemap, unsigned int width, unsigned int height, const char* hdrsavepath);
	unsigned int cubemap_to_equirectangular(unsigned int precomputemap, unsigned int width, unsigned int height);//��������İ汾
	void PrecomputeProcessPackHDR(const char* prefilterpackpath);//ר�Ű�prefilterת��texture atlas��ʽ
	void save_image(const char* filepath, int image_width, int image_height, int channels, const char* file_extension);
	void renderQuad();


public:
	//Ԥ��������LUTͼ
	unsigned int Precompute_brdfLUTTexture(Shader ourshader, const char* filename);
	unsigned int Precompute_IBLLightPosition(Shader ourshader, const char* IBLLightPosition);//��HDRͼƬ�������������ȡ������Ϊ��Դ


protected:
	// ֱ�Ӵ�hdrͼ�ϲ��� ������ת��cubemap�Ĺ���
	Shader hdrTextureShader = { "hdrtexture_vs.glsl", "hdrtexture_fs.glsl" };
	Shader hdrLightShader = { "hdrtexture_vs.glsl", "hdrtexture_light_fs.glsl" };
	Shader CubemapToequirectangularShader = { "hdrtexture_vs.glsl", "cubemap_to_equirectangular_fs.glsl" };

	unsigned int brdfLUT;
	unsigned int lightpositionMap;
	unsigned int hdrprefilterMap0, hdrprefilterMap1, hdrprefilterMap2, hdrprefilterMap3, hdrprefilterMap4;

protected:
	void PrefilterPackArrangement(float* data, unsigned int hdrprefilterMap, unsigned int PACK_HDR_TEXTURE, int Texwidth, int Texheight, int PosX, int PosY);

protected:
	int TextureWidth, TextureHeight, TextureChannels;

};


class Precompute : public CubemapPrecompute, public HDRPrecompute
{

public:
	void PrecomputeProcess(const char* hdr_filepath);//����cubemap��Ԥ����
	void PrecomputeProcessHDR(const char* irradiancepath, const char* prefilterpackpath);//������ʽ��Ԥ����
	void PrecomputeProcessLUT(const char* brdffilepath, const char* IBLLightPosition);//brdf������ͼ

};
