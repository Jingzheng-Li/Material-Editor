
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <core/GL_utils.h>
#include <core/model.h>

//预计算tools
#include <core/precompute.h>
//选择渲染模型：包括standard/cloth等模型
#include <core/shading_model.h>
//从资源文件夹中加载文件
#include <core/LoadFiles.h>
//渲染skybox大场景 同时把irradiance prefilter brdfLUT等渲染进去
#include <core/renderscene.h>


#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"


#include <stdio.h>
#include <iostream>


Camera camera(CameraPosition);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
bool Init_glad();
bool Init_window(GLFWwindow* window);
bool Init_imgui(GLFWwindow* window);
void mouse_callback_rotate_scene(GLFWwindow* window, int button, int action, int mods);
void obtain_rotate_angle(GLFWwindow* window);

StandardModelOptions StandardOptions;
ClothModelOptions ClothOptions;


// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;


//全局变量定义在globalVariables.h可以看到解释

const float PI = 3.14159265359;

//hdr and model files
vector<const char*> scenes;
vector<const char*> models;
vector<const char*> albedoMaps;
vector<const char*> metallicMaps;
vector<const char*> normalMaps;
vector<const char*> roughnessMaps;
vector<const char*> aoMaps;
vector<string> scenenames;

const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;
const unsigned int DEPTHMAP_WIDTH = 1024;
const unsigned int DEPTHMAP_HEIGHT = 1024;
const unsigned int maxMipLevels = 5;


const string precomputepath = FileSystem::getPath("resources/textures/precompute");
string irradianceMapHDR;
vector<string> prefilterMapHDR;
string prefilterMapHDRPack;
vector<string> envCubemapFaces;
vector<string> IrradCubemapFaces;
vector<string> PrefilterCubemapFaces;
string IBLLightPosition;

string brdffilepath = FileSystem::getPath("resources/textures/precompute/brdfLUTTexture.png");
const char* brdf_filepath = brdffilepath.c_str();


unsigned int captureFBO;
unsigned int captureRBO;
unsigned int depthMapFBO;

unsigned int hdrTexture;
unsigned int irrad_hdrTexture;
unsigned int prefil_hdrTexture;
unsigned int ibllightposition;
unsigned int envCubemap;
unsigned int irradianceMap;
unsigned int prefilterMap;
unsigned int brdfLUT;
unsigned int depthMap;
unsigned int frontdepthTexture;

//arcball相关参数
float rotateAngleX = 0.0;
float rotateAngleY = 0.0;
double lastX = 0;
double lastY = 0;
double curr_x = 0;
double curr_y = 0;
glm::vec2 last = glm::vec2(lastX, lastY);
bool left_button_down;
glm::vec3 CameraPosition = glm::vec3(0.0f, 0.0f, 5.0f);
const float arcball_radius = glm::length(CameraPosition);

int main()
{
	//initialize window/glad/imgui
	glfwInit();
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Material Editor", NULL, NULL);
	if (!Init_window(window)) return -1;
	if (!Init_glad()) return -1;
	if (!Init_imgui(window)) return -1;

	// Our state
	bool show_demo_window = false;
	bool point_light = false;
	bool area_light = false;

	//some useful const
	bool has_load_hdr = false;
	bool has_load_model = false;
	bool standard_model = true;
	bool cloth_model = false;

	//对应了最开始model hdrscene 各种texture的index
	int sampleSceneIndex = 4;
	int modelSceneIndex = 6;
	int albedoMapIndex = 0;
	int metallicMapIndex = 0;
	int normalMapIndex = 0;
	int roughnessMapIndex = 0;
	int aoMapIndex = 0;

	//从本地加载文件 包括hdr model 各种texture
	LoadFiles* loadfiles = NULL;
	//加载模型
	Model* IBLmodel = NULL;
	//基于hdrscene对场景进行预计算包括envcubemap irradiance prefilter LUT等
	Precompute envprecompute;
	//专门用来渲染场景
	RenderScene renderscene;
	//选择渲染的方式 可以是standard model，cloth model TODO: hair model skin model...
	ShadingModel shadingmodel;

	

	//Load all the model/hdr/texturemaps to different global files scenes/models/albedoMaps/metallicMaps...
	loadfiles = new LoadFiles;
	loadfiles->AddFilepath();

	//scenes models albedoMaps 对应了此类型所有图片的地址 har_filepath model_filepath从中选取一个地址
	char* hdr_filepath = (char*)scenes[sampleSceneIndex];
	char* model_filepath = (char*)models[modelSceneIndex];
	char* albedo_filepath = (char*)albedoMaps[albedoMapIndex];
	char* metallic_filepath = (char*)metallicMaps[metallicMapIndex];
	char* roughness_filepath = (char*)roughnessMaps[roughnessMapIndex];
	char* normal_filepath = (char*)normalMaps[normalMapIndex];
	char* ao_filepath = (char*)aoMaps[aoMapIndex];


	//初始化加载AlbedoID（用作imgui显示的预览图），后面在更换图片选项后需要重新loadTexture
	ImTextureID AlbedoID = (void*)loadTexture(albedo_filepath);
	ImTextureID MetallicID = (void*)loadTexture(metallic_filepath);
	ImTextureID RoughnessID = (void*)loadTexture(roughness_filepath);
	ImTextureID NormalID = (void*)loadTexture(normal_filepath);
	ImTextureID AoID = (void*)loadTexture(ao_filepath);


	//用作后面textureMapping使用的贴图，因此使用两种读取texture方式
	unsigned int albedoMap = loadTextureMaps(albedo_filepath);
	unsigned int metallicMap = loadTextureMaps(metallic_filepath);
	unsigned int roughnessMap = loadTextureMaps(roughness_filepath);
	unsigned int normalMap = loadTextureMaps(normal_filepath);
	unsigned int aoMap = loadTextureMaps(ao_filepath);


	// point lights 
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	

	// configure global opengl state
	glEnable(GL_DEPTH_TEST); // set depth function to less than AND equal for skybox depth trick.
	glDepthFunc(GL_LEQUAL); // enable seamless cubemap sampling for lower mip levels in the pre-filter map.
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); //eliminate the seam between box
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_CULL_FACE);//cull backface
	glCullFace(GL_BACK);



	//这些是renderCore的shader
	Shader pbrShader("pbr_vs.glsl", "pbr_fs.glsl");
	Shader ClothShader("pbr_vs.glsl", "cloth_pbr_fs.glsl");
	Shader backgroundShader("background_vs.glsl", "background_fs.glsl");
	Shader pointlightShader("point_light_vs.glsl", "point_light_fs.glsl");
	Shader depthShader("depth_vs.glsl", "depth_fs.glsl");

	//初始化shader 包括绑定 创建depthMap纹理等
	shadingmodel.SetIntShader(pbrShader, ClothShader, backgroundShader);

	//==================================================================================================================

	//代码测试区域 文件预加载在前面都已经完成 这里可以直接测试函数功能
	

	//==================================================================================================================
	//PRECOMPUTE TOOLS
	//会按照所有的hdr场景全部过滤一边图 如果不存在就重新计算 存在就跳过
	//预计算在这里暂时不参与考虑 全部先掠过

	cout << "intializing... please waiting... " << endl;

	for (unsigned int sceneIndex = 0; sceneIndex < scenenames.size(); ++sceneIndex)
	{

		envCubemapFaces = ModifyFilepath("envCubeMap", scenenames[sceneIndex]);
		IrradCubemapFaces = ModifyFilepath("irradianceMap", scenenames[sceneIndex]);
		PrefilterCubemapFaces = ModifyFilepath("prefilterMap", scenenames[sceneIndex]);

		irradianceMapHDR = ModifyFilepath("irradianceMapHDR", scenenames[sceneIndex])[0];
		prefilterMapHDR = ModifyFilepath("prefilterMapHDR", scenenames[sceneIndex]);
		prefilterMapHDRPack = ModifyFilepath("prefilterMapHDRPack", scenenames[sceneIndex])[0];

		IBLLightPosition = ModifyFilepath("IBLLightPosition", scenenames[sceneIndex])[0];

		// 预计算的核心过程 会检测所有的envCubemap irradCubemap prefilterCubemap 这四类预计算图 补全缺失
		//envprecompute.PrecomputeProcess(scenes[sceneIndex]);
		// 检测packhdr图是否完整 预计算pack hdr图片
		//envprecompute.PrecomputeProcessHDR(irradianceMapHDR.c_str(), prefilterMapHDRPack.c_str());
		// 计算brdfLUT 和 IBL光源方向及色彩
		//envprecompute.PrecomputeProcessLUT(brdffilepath.c_str(), IBLLightPosition.c_str());

		cout << scenes[sceneIndex] << endl;

	}

	cout << "intializing finished " << endl;

	//==================================================================================================================




	// then before rendering, configure the viewport to the original framebuffer's screen dimensions
	int scrWidth, scrHeight;
	float near_plane = 1.0f, far_plane = 7.5f;
	glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	glViewport(0, 0, scrWidth, scrHeight);
	glDeleteFramebuffers(1, &captureFBO);


	// render loop
	// -----------------------------------------------------------------------------------
	while (!glfwWindowShouldClose(window))
	{

		processInput(window);
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		//通过glfw获取屏幕空间移动距离，并且触发arcball
		obtain_rotate_angle(window);


		//场景切换 根据hdr对应名称选择precompute信息 然后进行场景渲染
		if (has_load_hdr == false)
		{

			envCubemapFaces = ModifyFilepath("envCubeMap", scenenames[sampleSceneIndex]);
			IrradCubemapFaces = ModifyFilepath("irradianceMap", scenenames[sampleSceneIndex]);
			PrefilterCubemapFaces = ModifyFilepath("prefilterMap", scenenames[sampleSceneIndex]);

			irradianceMapHDR = ModifyFilepath("irradianceMapHDR", scenenames[sampleSceneIndex])[0];
			prefilterMapHDRPack = ModifyFilepath("prefilterMapHDRPack", scenenames[sampleSceneIndex])[0];
			IBLLightPosition = ModifyFilepath("IBLLightPosition", scenenames[sampleSceneIndex])[0];



			//使用不同的采样方法采样prefilter等
#ifdef TEXTURE_WITH_ENVCUBEMAP

			//======================================================================
			//临时加上 因为background使用的是按照hdr采样 没有按照cubemap采样 改变hdrTexture 为了能画出background 等下删掉 并且修改background_fs.glsl能得到相同的效果
			hdrTexture = loadTexturef(hdr_filepath);
			//======================================================================
			renderscene.render_hdrscene(envCubemapFaces, IrradCubemapFaces, PrefilterCubemapFaces);

#endif 


#ifdef TEXTURE_WITH_HDR

			renderscene.render_hdrscene(hdr_filepath, irradianceMapHDR.c_str(), prefilterMapHDRPack.c_str(), IBLLightPosition.c_str());

#endif 

			cout << "load hdr_filepath:" << "  " << hdr_filepath << endl;

			has_load_hdr = true;
		}

		if (has_load_model == false)
		{
			delete IBLmodel;
			IBLmodel = new Model;
			//这部分load会形成mesh
			IBLmodel->Load(model_filepath);
			cout << "test model_filepath" << "  " << model_filepath << endl;
			has_load_model = true;

		}


		//imgui part
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();


		if (show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);

		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		{

			ImGui::Begin("Material Editor");

			if (ImGui::RadioButton("Standard", standard_model == true)) { standard_model = true, cloth_model = false; }
			ImGui::SameLine();
			if (ImGui::RadioButton("Cloth", cloth_model == true)) { standard_model = false, cloth_model = true; }


			if (standard_model) {
				ImGui::Text("standard model parameters");     // Display some text (you can use a format strings too)
				ImGui::ColorEdit3("albedo", (float*)&StandardOptions.albedo);
				ImGui::SliderFloat("metallic", &StandardOptions.metallic, 0.0f, 2.0f);
				ImGui::SliderFloat("roughness", &StandardOptions.roughness, 0.0f, 2.0f);
				ImGui::SliderFloat("reflectance", &StandardOptions.reflectance, 0.0f, 1.0f);
				ImGui::SliderFloat("sheen", &StandardOptions.sheen, 0.0f, 2.0f);
				ImGui::SliderFloat("sheenTint", &StandardOptions.sheenTint, 0.0f, 1.0f);
				ImGui::SliderFloat("clearcoat", &StandardOptions.clearcoat, 0.0f, 1.0f);
				ImGui::SliderFloat("clearcoatRoughness", &StandardOptions.clearcoatRoughness, 0.0f, 1.0f);
				ImGui::SliderFloat("subsurfacePower", &StandardOptions.subsurfacePower, 0.0f, 1.0f);
				ImGui::SliderFloat("subsurfaceScale", &StandardOptions.subsurfaceScale, 0.0f, 1.0f);
				ImGui::SliderFloat("thicknessScale", &StandardOptions.thicknessScale, 0.0f, 1.0f);
				ImGui::SliderFloat("anisotropy", &StandardOptions.anisotropy, -1.0f, 1.0f);
				ImGui::SliderFloat("env_brightness", &StandardOptions.env_brightness, 0.5f, 2.5f);

			}

			if (cloth_model) {
				ImGui::Text("cloth model parameters");  // Display some text (you can use a format strings too)
				ImGui::ColorEdit3("albedo_cloth", (float*)&ClothOptions.albedo);
				ImGui::ColorEdit3("sheenColor_cloth", (float*)&ClothOptions.sheenColor);
				ImGui::ColorEdit3("subsurfaceColor_cloth", (float*)&ClothOptions.subsurfaceColor);
				ImGui::SliderFloat("roughness_cloth", &ClothOptions.roughness, 0.0f, 1.0f);
				ImGui::SliderFloat("fabricscatter_cloth", &ClothOptions.fabricscatter, 0.0f, 1.0f);
				ImGui::SliderFloat("sheen_cloth", &ClothOptions.sheen, 0.0f, 1.0f);
				ImGui::SliderFloat("sheenTint_cloth", &ClothOptions.sheenTint, 0.0f, 1.0f);
				ImGui::SliderFloat("reflectance_cloth", &ClothOptions.reflectance, 0.4f, 0.8f);
				ImGui::SliderFloat("env_brightness_cloth", &ClothOptions.env_brightness, 0.5f, 2.5f);
			}




			ImGui::Separator();

			if (ImGui::Combo("HDR scene", &sampleSceneIndex, scenes.data(), scenes.size()))
			{
				hdr_filepath = (char*)scenes[sampleSceneIndex];
				has_load_hdr = false;
			}


			if (ImGui::Combo("Models", &modelSceneIndex, models.data(), models.size()))
			{
				model_filepath = (char*)models[modelSceneIndex];
				has_load_model = false;
			}




			ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
			ImGui::Checkbox("point light", &point_light);
			ImGui::Checkbox("area light", &area_light);



			//Transforms Properties
			ImGui::Separator();


			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}

		// 3. point window.
		if (point_light)
		{
			ImGui::SetCursorPos(ImVec2(10.0, 30.0));
			ImGui::Begin("Point light", &point_light);
			ImGui::Text("Point light making");
			ImGui::SliderFloat3("point light pos", &StandardOptions.PointlightPositions.x, -10.0, 10.0);
			ImGui::ColorEdit3("point light color", (float*)&StandardOptions.PointlightColors);
			if (ImGui::Button("Close Me"))
				point_light = false;
			ImGui::End();
		}

		// 4. aera light
		if (area_light)
		{
			ImGui::Begin("Area light", &area_light);
			ImGui::Text("Area light making");
			if (ImGui::Button("Close Me"))
				area_light = false;
			ImGui::End();
		}



		//texture maps
		if (standard_model)
		{
			ImGui::Begin("use textures");


			//这里显色错误实际上是因为图片的位数问题，8/16/24/32对应R/RG/RGB/RGBA
			//显示红色什么的是因为以为通道只选择了RED

			if (ImGui::Combo("Albedo Map", &albedoMapIndex, albedoMaps.data(), albedoMaps.size()))
			{
				albedo_filepath = (char*)albedoMaps[albedoMapIndex];
				loadfiles->albedo_change = true;
			}
			if (loadfiles->albedo_change)
			{
				AlbedoID = (void*)loadTexture(albedo_filepath);
				albedoMap = loadTextureMaps(albedo_filepath);
				loadfiles->albedo_change = false;
			}
			ImGui::Image(AlbedoID, ImVec2(100, 100));
			ImGui::Separator();



			if (ImGui::Combo("Metallic Map", &metallicMapIndex, metallicMaps.data(), metallicMaps.size()))
			{

				metallic_filepath = (char*)metallicMaps[metallicMapIndex];
				loadfiles->metallic_change = true;
			}
			if (loadfiles->metallic_change)
			{
				MetallicID = (void*)loadTexture(metallic_filepath);
				metallicMap = loadTextureMaps(metallic_filepath);
				loadfiles->metallic_change = false;
			}
			ImGui::Image(MetallicID, ImVec2(100, 100));
			ImGui::Separator();


			if (ImGui::Combo("Roughness Map", &roughnessMapIndex, roughnessMaps.data(), roughnessMaps.size()))
			{
				roughness_filepath = (char*)roughnessMaps[roughnessMapIndex];
				loadfiles->roughness_change = true;
			}
			if (loadfiles->roughness_change)
			{
				RoughnessID = (void*)loadTexture(roughness_filepath);
				roughnessMap = loadTextureMaps(roughness_filepath);
				loadfiles->roughness_change = false;
			}
			ImGui::Image(RoughnessID, ImVec2(100, 100));
			ImGui::Separator();


			if (ImGui::Combo("Normal Map", &normalMapIndex, normalMaps.data(), normalMaps.size()))
			{
				normal_filepath = (char*)normalMaps[normalMapIndex];
				loadfiles->normal_change = true;
			}
			if (loadfiles->normal_change)
			{
				NormalID = (void*)loadTexture(normal_filepath);
				normalMap = loadTextureMaps(normal_filepath);
				loadfiles->normal_change = false;
			}
			ImGui::Image(NormalID, ImVec2(100, 100));
			ImGui::Separator();



			if (ImGui::Combo("Ao Map", &aoMapIndex, aoMaps.data(), aoMaps.size()))
			{
				ao_filepath = (char*)aoMaps[aoMapIndex];
				loadfiles->ao_change = true;
			}
			if (loadfiles->ao_change)
			{
				AoID = (void*)loadTexture(ao_filepath);
				aoMap = loadTextureMaps(ao_filepath);
				loadfiles->ao_change = false;
			}
			ImGui::Image(AoID, ImVec2(100, 100));



			ImGui::End();
		}

		//texture maps
		if (cloth_model)
		{
			ImGui::Begin("use textures");


			//这里显色错误实际上是因为图片的位数问题，8/16/24/32对应R/RG/RGB/RGBA
			//显示红色什么的是因为以为通道只选择了RED

			if (ImGui::Combo("Albedo Map", &albedoMapIndex, albedoMaps.data(), albedoMaps.size()))
			{
				albedo_filepath = (char*)albedoMaps[albedoMapIndex];
				loadfiles->albedo_change = true;
			}
			if (loadfiles->albedo_change)
			{
				AlbedoID = (void*)loadTexture(albedo_filepath);
				albedoMap = loadTextureMaps(albedo_filepath);
				loadfiles->albedo_change = false;
			}
			ImGui::Image(AlbedoID, ImVec2(100, 100));
			ImGui::Separator();


			if (ImGui::Combo("Roughness Map", &roughnessMapIndex, roughnessMaps.data(), roughnessMaps.size()))
			{
				roughness_filepath = (char*)roughnessMaps[roughnessMapIndex];
				loadfiles->roughness_change = true;
			}
			if (loadfiles->roughness_change)
			{
				RoughnessID = (void*)loadTexture(roughness_filepath);
				roughnessMap = loadTextureMaps(roughness_filepath);
				loadfiles->roughness_change = false;
			}
			ImGui::Image(RoughnessID, ImVec2(100, 100));
			ImGui::Separator();


			if (ImGui::Combo("Normal Map", &normalMapIndex, normalMaps.data(), normalMaps.size()))
			{
				normal_filepath = (char*)normalMaps[normalMapIndex];
				loadfiles->normal_change = true;
			}
			if (loadfiles->normal_change)
			{
				NormalID = (void*)loadTexture(normal_filepath);
				normalMap = loadTextureMaps(normal_filepath);
				loadfiles->normal_change = false;
			}
			ImGui::Image(NormalID, ImVec2(100, 100));
			ImGui::Separator();



			if (ImGui::Combo("Ao Map", &aoMapIndex, aoMaps.data(), aoMaps.size()))
			{
				ao_filepath = (char*)aoMaps[aoMapIndex];
				loadfiles->ao_change = true;
			}
			if (loadfiles->ao_change)
			{
				AoID = (void*)loadTexture(ao_filepath);
				aoMap = loadTextureMaps(ao_filepath);
				loadfiles->ao_change = false;
			}
			ImGui::Image(AoID, ImVec2(100, 100));



			ImGui::End();
		}
		

		//生成lightSpace下面的深度图
		glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);//投影
		glm::mat4 lightView = glm::lookAt(StandardOptions.PointlightPositions, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
		glm::mat4 lightSpaceMatrix = lightProjection * lightView;
		glViewport(0, 0, DEPTHMAP_WIDTH, DEPTHMAP_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		depthShader.use();
		depthShader.setMat4("model", glm::mat4(1.0f));
		depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
		IBLmodel->Draw(depthShader);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);



		//========================================================================================
		
		glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
		glViewport(0, 0, scrWidth, scrHeight);
		glClearColor(clear_color.x* clear_color.w, clear_color.y* clear_color.w, clear_color.z* clear_color.w, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//rendering core 渲染的核心部分 根据之前model定义的mesh，结合shader和texture进行绘制	
		if (standard_model)
		{
			//shadingmodel是为Draw完成纹理绑定激活等处理
			shadingmodel.Standard_model(pbrShader, albedoMap, metallicMap, roughnessMap, normalMap, aoMap);
			IBLmodel->Draw(pbrShader);
		}
		else if (cloth_model)
		{
			shadingmodel.Cloth_model(ClothShader, albedoMap, roughnessMap, normalMap, aoMap);
			IBLmodel->Draw(ClothShader);
		}

		//render point light
		if (point_light)
		{
			shadingmodel.RenderPointLight(pointlightShader);
			renderSphere();
		}

		// render skybox (render as last to prevent overdraw)
		renderscene.Draw_background(backgroundShader);


		// render BRDF LUT  to screen 
		//brdfShader.use();
		//renderQuad();


		//render imgui
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		glfwPollEvents();
		glfwSwapBuffers(window);

	}


	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	delete loadfiles;
	glDeleteFramebuffers(1, &captureFBO);

	// glfw: terminate, clearing all previously allocated GLFW resources.
	glfwTerminate();
	return 0;

}





// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}



void mouse_callback_rotate_scene(GLFWwindow* window, int button, int action, int mods)
{
	//GLFW_PRESS是一个瞬时的行为，这个函数本身是为了获得last的坐标，每次获得的last坐标是鼠标点击的位置
	//并且诊断到left_button_down，让获得角度的函数obtian_rotate_angle可以运行

	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		if (action == GLFW_PRESS)
		{
			//cout << "button clicked" << endl;
			glfwGetCursorPos(window, &curr_x, &curr_y);
			last = glm::vec2(curr_x, curr_y);
			left_button_down = true;
		}
		else if (action == GLFW_RELEASE)
			left_button_down = false;
	}

}

//滑动会产生距离，这个函数记录了鼠标滑动过程中距离
void obtain_rotate_angle(GLFWwindow* window)
{
	if (left_button_down) {
		glfwGetCursorPos(window, &curr_x, &curr_y);
		double xoffset = curr_x - last.x;
		double yoffset = curr_y - last.y;
		rotateAngleX = xoffset;
		rotateAngleY = yoffset;
		last.x = curr_x;
		last.y = curr_y;
	}
	//快速拖拽会出现残余的rotateAngleX和Y，不会立即为零，会很流畅
	if (rotateAngleX != 0 || rotateAngleY != 0)
	{
		camera.ProcessMouseMovement(rotateAngleX, rotateAngleY, arcball_radius, true);
		rotateAngleX *= 0.95, rotateAngleY *= 0.95;
		if (rotateAngleX < 0.1) rotateAngleX = 0;
		if (rotateAngleY < 0.1) rotateAngleY = 0;
		//cout << rotateAngleX << " " << rotateAngleY << endl;
	}

}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

bool Init_glad()
{
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return false;
	}

	return true;
}

bool Init_window(GLFWwindow* window)
{

	// glfw: initialize and configure
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//#ifdef __APPLE__
	//	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	//#endif
	glfwMakeContextCurrent(window);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_callback_rotate_scene);

	//setInt camera
	camera.Radius = arcball_radius;
	camera.ProcessMouseMovement(rotateAngleX, rotateAngleY, arcball_radius, true);

	return true;
}

bool Init_imgui(GLFWwindow* window)
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	// Setup Dear ImGui style
	ImGui::StyleColorsLight();
	// Setup Platform/Renderer backends
	const char* glsl_version = "#version 130";
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	return true;
}








