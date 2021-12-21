

#include <core/renderscene.h>

extern Camera camera;

void RenderScene::render_hdrscene(vector<string> envCubemapFace, vector<string> IrradCubemapFace, vector<string> PrefilterCubemapFace)
{

	//所有的textureID都是加载的faces，是修改过路径的texture
	//render envCubemap
	//这个是用来给后面render background使用的
	envCubemap = loadCubemap(envCubemapFace);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);


	//render irradmap
	irradianceMap = loadCubemap(IrradCubemapFace);
	glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);


	//render prefiltermap
	prefilterMap = loadCubemap_MipMap(PrefilterCubemapFace);
	glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	//render brdfLUT
	brdfLUT = loadTexture(brdf_filepath);



}

void RenderScene::render_hdrscene(const char* hdr_filepath, const char* irrad_hdr_filepath, const char* prefil_hdr_filepath, const char* ibl_lightposition_filepath)
{

	//随着场景改变 hdr的filepath也要改变
	hdrTexture = loadTexturef(hdr_filepath);
	glBindTexture(GL_TEXTURE_2D, hdrTexture);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	irrad_hdrTexture = loadTexturef(irrad_hdr_filepath);
	glBindTexture(GL_TEXTURE_2D, irrad_hdrTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	//在loadtexturef_PackMipMap里面用cpu分割了一下
	prefil_hdrTexture = loadTexturef_PackMipMap(prefil_hdr_filepath);
	glBindTexture(GL_TEXTURE_2D, prefil_hdrTexture);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	ibllightposition = loadTexturef(ibl_lightposition_filepath);
	glBindTexture(GL_TEXTURE_2D, ibllightposition);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	//render brdfLUT
	brdfLUT = loadTexture(brdf_filepath);
	


}


void RenderScene::Draw_background(Shader backgroundShader)
{
	// render skybox (render as last to prevent overdraw)
	backgroundShader.use();

	projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	view = camera.GetViewMatrix();
	model = glm::mat4(1.0);

	backgroundShader.setMat4("model", model);
	backgroundShader.setMat4("view", view);
	backgroundShader.setMat4("projection", projection);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
	//glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap); // display irradiance map
	//glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap); // display prefilter map

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, hdrTexture);

	renderCube();

}

