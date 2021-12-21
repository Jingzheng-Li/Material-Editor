#pragma once

#include <core/GL_utils.h>



struct StandardModelOptions
{
	StandardModelOptions()
	{
		albedo = glm::vec3(1.0, 1.0, 1.0);
		metallic = 1.0f;
		roughness = 1.0f;
		reflectance = 0.04f;
		sheen = 0.0f;
		sheenTint = 0.0f;
		clearcoat = 0.0f;
		clearcoatRoughness = 0.0f;
		anisotropy = 0.0f;
		env_brightness = 1.0f;
		subsurfacePower = 0.0f;
		subsurfaceScale = 0.0f;
		thicknessScale = 0.5f;
		PointlightPositions = glm::vec3(0.0f, 0.0f, -5.0f);
		PointlightColors = glm::vec3(0.5f, 0.5f, 0.5f);

	}
	glm::vec3 albedo;
	float metallic;
	float roughness;
	float reflectance;
	float sheen;
	float sheenTint;
	float clearcoat;
	float clearcoatRoughness;
	float anisotropy;
	float env_brightness;
	float subsurfacePower;
	float subsurfaceScale;
	float thicknessScale;
	glm::vec3 PointlightPositions;
	glm::vec3 PointlightColors;

};

struct ClothModelOptions
{
	ClothModelOptions()
	{
		albedo = glm::vec3(1.0, 1.0, 1.0);
		sheenColor = glm::vec3(1.0, 1.0, 1.0);
		subsurfaceColor = glm::vec3(1.0, 1.0, 1.0);
		roughness = 0.0;
		fabricscatter = 0.5;
		sheen = 0.0;
		sheenTint = 0.0;
		reflectance = 0.4;
		env_brightness = 1.0;
	}
	glm::vec3 albedo;
	glm::vec3 sheenColor;
	glm::vec3 subsurfaceColor;
	float roughness;
	float fabricscatter;
	float sheen;
	float sheenTint;
	float reflectance;
	float env_brightness;

};

class ShadingModel
{
public:
	ShadingModel() {};
	~ShadingModel() {};
	
	//给shader初始化
	void SetIntShader(Shader pbrShader, Shader ClothShader, Shader backgroundShader);

	//几种不同的模型 包括standard等
	void Standard_model(Shader pbrShader, unsigned int albedoMap, unsigned int metallicMap, unsigned int roughnessMap, unsigned int normalMap, unsigned int aoMap);
	//TODO cloth应该按照prefilter_cloth_fs重新生成一个prefilter 现在是直接用的irradiancemap 等下需要重新生成
	void Cloth_model(Shader ClothShader, unsigned int albedoMap, unsigned int roughnessMap, unsigned int normalMap, unsigned int aoMap);
	void Skin_model();
	void Eye_model();
	void Hair_model();

	void RenderPointLight(Shader pointlightShader);


private:
	glm::mat4 projection;
	glm::mat4 view;
	glm::mat4 model;
	glm::mat4 lightProjection;
	glm::mat4 lightView;
	glm::mat4 lightSpaceMatrix;
	float near_plane = 1.0f, far_plane = 7.5f;
	

	
	
};


