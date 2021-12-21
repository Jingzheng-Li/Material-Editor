
#include <core/shading_model.h>

extern Camera camera;
extern StandardModelOptions StandardOptions;
extern ClothModelOptions ClothOptions;

void ShadingModel::SetIntShader(Shader pbrShader, Shader ClothShader, Shader backgroundShader)
{

	pbrShader.use();
	pbrShader.setInt("irradianceMap", 0);
	pbrShader.setInt("brdfLUT", 1);
	pbrShader.setInt("prefilterMap", 2);
	//TextureMaps
	pbrShader.setInt("albedoMap", 5);
	pbrShader.setInt("metallicMap", 6);
	pbrShader.setInt("roughnessMap", 7);
	pbrShader.setInt("normalMap", 8);
	pbrShader.setInt("aoMap", 9);
	//HDR
	pbrShader.setInt("equirectangularMap", 10);
	pbrShader.setInt("Irradiance_equirectangularMap", 11);
	pbrShader.setInt("Prefilter_equirectangularMap", 12);
	//Subsurface
	pbrShader.setInt("IBLLightPosition", 13);
	pbrShader.setInt("depthMap", 14);
	pbrShader.setInt("frontdepthMap", 15);


	ClothShader.use();
	ClothShader.setInt("irradianceMap", 0);
	ClothShader.setInt("brdfLUT", 1);
	ClothShader.setInt("prefilterMap", 2);
	//TextureMaps
	ClothShader.setInt("albedoMap", 3);
	ClothShader.setInt("roughnessMap", 5);
	ClothShader.setInt("normalMap", 6);
	ClothShader.setInt("aoMap", 7);
	//HDR
	ClothShader.setInt("equirectangularMap", 10);
	ClothShader.setInt("Irradiance_equirectangularMap", 11);
	ClothShader.setInt("Prefilter_equirectangularMap", 12);

	//render skybox
	backgroundShader.use();
	backgroundShader.setInt("environmentMap", 0);
	backgroundShader.setInt("equirectangularMap", 1);


	//create depth texture and bind to depthMapFBO
	glGenFramebuffers(1, &depthMapFBO);
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, DEPTHMAP_WIDTH, DEPTHMAP_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}



void ShadingModel::Standard_model(Shader pbrShader, unsigned int albedoMap, unsigned int metallicMap, unsigned int roughnessMap, unsigned int normalMap, unsigned int aoMap)
{
	projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	view = camera.GetViewMatrix();
	model = glm::mat4(1.0f);
	lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
	lightView = glm::lookAt(StandardOptions.PointlightPositions, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
	lightSpaceMatrix = lightProjection * lightView;

	pbrShader.use();

	pbrShader.setMat4("projection", projection);
	pbrShader.setMat4("view", view);
	pbrShader.setMat4("model", model);
	pbrShader.setVec3("camPos", camera.Position);
	pbrShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

	pbrShader.setVec3("albedo", StandardOptions.albedo);
	pbrShader.setFloat("metallic", StandardOptions.metallic);
	pbrShader.setFloat("roughness", StandardOptions.roughness);
	pbrShader.setFloat("reflectance", StandardOptions.reflectance);
	pbrShader.setFloat("sheen", StandardOptions.sheen);
	pbrShader.setFloat("sheenTint", StandardOptions.sheenTint);
	pbrShader.setFloat("clearcoat", StandardOptions.clearcoat);
	pbrShader.setFloat("clearcoatRoughness", StandardOptions.clearcoatRoughness);
	pbrShader.setFloat("anisotropy", StandardOptions.anisotropy);
	pbrShader.setFloat("env_brightness", StandardOptions.env_brightness);
	pbrShader.setFloat("subsurfacePower", StandardOptions.subsurfacePower);
	pbrShader.setFloat("thicknessScale", StandardOptions.thicknessScale);
	pbrShader.setFloat("subsurfaceScale", StandardOptions.subsurfaceScale);
	pbrShader.setVec3("PointlightPositions", StandardOptions.PointlightPositions);
	pbrShader.setVec3("PointlightColors", StandardOptions.PointlightColors);


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, brdfLUT);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);


	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, albedoMap);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, metallicMap);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, roughnessMap);
	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, normalMap);
	glActiveTexture(GL_TEXTURE9);
	glBindTexture(GL_TEXTURE_2D, aoMap);


	//直接按照hdrtexture采样时使用
	glActiveTexture(GL_TEXTURE10);
	glBindTexture(GL_TEXTURE_2D, hdrTexture);
	glActiveTexture(GL_TEXTURE11);
	glBindTexture(GL_TEXTURE_2D, irrad_hdrTexture);
	glActiveTexture(GL_TEXTURE12);
	glBindTexture(GL_TEXTURE_2D, prefil_hdrTexture);


	glActiveTexture(GL_TEXTURE13);
	glBindTexture(GL_TEXTURE_2D, ibllightposition);
	glActiveTexture(GL_TEXTURE14);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glActiveTexture(GL_TEXTURE15);
	glBindTexture(GL_TEXTURE15, frontdepthTexture);

}



void ShadingModel::Cloth_model(Shader ClothShader, unsigned int albedoMap, unsigned int roughnessMap, unsigned int normalMap, unsigned int aoMap)
{

	ClothShader.use();

	projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	view = camera.GetViewMatrix();
	model = glm::mat4(1.0f);

	ClothShader.setVec3("camPos", camera.Position);
	ClothShader.setMat4("view", view);
	ClothShader.setMat4("model", model);
	ClothShader.setMat4("projection", projection);

	ClothShader.setVec3("albedo", ClothOptions.albedo);
	ClothShader.setVec3("sheenColor", ClothOptions.sheenColor);
	ClothShader.setVec3("subsurfaceColor", ClothOptions.subsurfaceColor);
	ClothShader.setFloat("roughness", ClothOptions.roughness);
	ClothShader.setFloat("fabricscatter", ClothOptions.fabricscatter);
	ClothShader.setFloat("sheen", ClothOptions.sheen);
	ClothShader.setFloat("sheenTint", ClothOptions.sheenTint);
	ClothShader.setFloat("env_brightness", ClothOptions.env_brightness);


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, brdfLUT);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);


	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, albedoMap);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, roughnessMap);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, normalMap);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, aoMap);

	//texture with equirectangularMap
	glActiveTexture(GL_TEXTURE10);
	glBindTexture(GL_TEXTURE_2D, hdrTexture);
	glActiveTexture(GL_TEXTURE11);
	glBindTexture(GL_TEXTURE_2D, irrad_hdrTexture);
	glActiveTexture(GL_TEXTURE12);
	glBindTexture(GL_TEXTURE_2D, prefil_hdrTexture);

}


void ShadingModel::RenderPointLight(Shader pointlightShader)
{

	projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	view = camera.GetViewMatrix();
	model = glm::mat4(1.0f);
	model = glm::translate(model, StandardOptions.PointlightPositions);
	model = glm::scale(model, glm::vec3(0.1f));

	pointlightShader.use();
	pointlightShader.setVec3("PointlightColors", StandardOptions.PointlightColors);
	pointlightShader.setMat4("projection", projection);
	pointlightShader.setMat4("view", view);
	pointlightShader.setMat4("model", model);

}



