
//预计算TOOLS部分 计算
#pragma once

#include <core/GL_utils.h>


class CubemapPrecompute
{

public:
	CubemapPrecompute() {};
	~CubemapPrecompute() {};

public:
	//将hdr转变为cubemap的precompute process 
	unsigned int equirectangular_to_cubemap(Shader ourshader, unsigned int hdrTexture);
	unsigned int PrecomputeIrradianceMap(Shader ourshader, unsigned int irradianceMaptest);
	unsigned int PrecomputePrefilterMap(Shader ourshader, unsigned int prefilterMap, unsigned int mip);
	void renderCube();
	unsigned int loadTexturef(const char* path);


protected:

	// 将hdr图转化成cubemap图片
	Shader equirectangularToCubemapShader = { "cubemap_vs.glsl", "equirectangular_to_cubemap_fs.glsl" };
	Shader irradianceShader = { "cubemap_vs.glsl", "irradiance_convolution_fs.glsl" };
	Shader prefilterShader = { "cubemap_vs.glsl", "prefilter_fs.glsl" };


	//生成brdf的LUT
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

	//直接从hdr采样的precompute process
	unsigned int cubemap_to_equirectangular(unsigned int precomputemap, unsigned int width, unsigned int height, const char* hdrsavepath);
	unsigned int cubemap_to_equirectangular(unsigned int precomputemap, unsigned int width, unsigned int height);//不带保存的版本
	void PrecomputeProcessPackHDR(const char* prefilterpackpath);//专门把prefilter转成texture atlas形式
	void save_image(const char* filepath, int image_width, int image_height, int channels, const char* file_extension);
	void renderQuad();


public:
	//预计算两个LUT图
	unsigned int Precompute_brdfLUTTexture(Shader ourshader, const char* filename);
	unsigned int Precompute_IBLLightPosition(Shader ourshader, const char* IBLLightPosition);//把HDR图片里面高亮部分提取出来作为光源


protected:
	// 直接从hdr图上采样 不经过转换cubemap的过程
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
	void PrecomputeProcess(const char* hdr_filepath);//给出cubemap的预计算
	void PrecomputeProcessHDR(const char* irradiancepath, const char* prefilterpackpath);//给出正式的预计算
	void PrecomputeProcessLUT(const char* brdffilepath, const char* IBLLightPosition);//brdf积分贴图

};
