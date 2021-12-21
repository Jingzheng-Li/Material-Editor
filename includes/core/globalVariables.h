#pragma once

//不要在这里放全局变量 一个都不要放

//三种采样方式 分别是hdr转换成的cubemap 直接在hdr上采样 和在pack_hdr上采样
// 在修改这个选项的同时 pbr_fs和background_fs也要重新定义
//#define TEXTURE_WITH_ENVCUBEMAP
#define TEXTURE_WITH_HDR


//包含全局变量和常用的包 被global_varables include 只用include utils就可以了

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <string>
#include <iostream>

#include <core/filesystem.h>
#include <core/camera.h>
#include <core/shader.h>

using namespace std;

extern const float PI;

//screen的长度和宽度
extern const unsigned int SCR_WIDTH;
extern const unsigned int SCR_HEIGHT;
extern const unsigned int DEPTHMAP_WIDTH;
extern const unsigned int DEPTHMAP_HEIGHT;

//prefilter的最大mipmap层数
extern const unsigned int maxMipLevels;

//视角变换辅助矩阵
extern glm::mat4 captureProjection;
extern glm::mat4 captureViews[];
//FBO和RBO
extern unsigned int captureFBO;
extern unsigned int captureRBO;
extern unsigned int depthMapFBO;

//包括所有场景，模型，贴图的路径所在
extern vector<const char*> scenes;
extern vector<const char*> models;
extern vector<const char*> albedoMaps;
extern vector<const char*> metallicMaps;
extern vector<const char*> roughnessMaps;
extern vector<const char*> normalMaps;
extern vector<const char*> aoMaps;

//所有场景的不带后缀不带前缀的名字 eg:cirus, door, factory, helipad 
extern vector<string> scenenames;

extern const string precomputepath;
extern string irradianceMapHDR;


//包含每个scenenames的cubemap路径 eg:"resources/textures/precompute/helipad/envCubeMap0.png"
//这个是根据scene变化而变化的量
extern vector<string> envCubemapFaces;
extern vector<string> IrradCubemapFaces;
extern vector<string> PrefilterCubemapFaces;

//两个预计算LUT的路径
extern const char* brdf_filepath;

//以hdr采样下的textureID
extern unsigned int hdrTexture;
extern unsigned int irrad_hdrTexture;
extern unsigned int prefil_hdrTexture;
extern unsigned int ibllightposition;

//以cubemap采样情况下的 所有预计算部分的textureID
extern unsigned int envCubemap;
extern unsigned int irradianceMap;
extern unsigned int prefilterMap;

//两个LUT的textureID 独立于采样
extern unsigned int brdfLUT;
extern unsigned int depthMap;
extern unsigned int frontdepthTexture;


//Camera的信息 包括position 定义camera 坐标轴方向如下
//          ^ 
//		   y|   /
//			|  /
//			| /
//------------------------ >x
//		   /| 
//		  / | 
//		 /  | 
//	   z/   |

extern glm::vec3 CameraPosition;
extern const float arcball_radius;







