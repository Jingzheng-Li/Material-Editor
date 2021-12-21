
//渲染场景 可以根据宏进行选择是使用cubemap还是使用hdr

#pragma once

#include <core/GL_utils.h>


class RenderScene
{
public:
	RenderScene() {};
	~RenderScene() {};

	//采样cubemap
	void render_hdrscene(vector<string> envCubemapFace, vector<string> IrradCubemapFace, vector<string> PrefilterCubemapFace);
	//采样hdr
	void render_hdrscene(const char* hdr_filepath, const char* irrad_hdr_filepath, const char* prefil_hdr_filepath, const char* ibl_lightposition_filepath);
	void Draw_background(Shader backgroundShader);

private:
	glm::mat4 projection;
	glm::mat4 view;
	glm::mat4 model;

};



