#pragma once

//��Ҫ�������ȫ�ֱ��� һ������Ҫ��

//���ֲ�����ʽ �ֱ���hdrת���ɵ�cubemap ֱ����hdr�ϲ��� ����pack_hdr�ϲ���
// ���޸����ѡ���ͬʱ pbr_fs��background_fsҲҪ���¶���
//#define TEXTURE_WITH_ENVCUBEMAP
#define TEXTURE_WITH_HDR


//����ȫ�ֱ����ͳ��õİ� ��global_varables include ֻ��include utils�Ϳ�����

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

//screen�ĳ��ȺͿ��
extern const unsigned int SCR_WIDTH;
extern const unsigned int SCR_HEIGHT;
extern const unsigned int DEPTHMAP_WIDTH;
extern const unsigned int DEPTHMAP_HEIGHT;

//prefilter�����mipmap����
extern const unsigned int maxMipLevels;

//�ӽǱ任��������
extern glm::mat4 captureProjection;
extern glm::mat4 captureViews[];
//FBO��RBO
extern unsigned int captureFBO;
extern unsigned int captureRBO;
extern unsigned int depthMapFBO;

//�������г�����ģ�ͣ���ͼ��·������
extern vector<const char*> scenes;
extern vector<const char*> models;
extern vector<const char*> albedoMaps;
extern vector<const char*> metallicMaps;
extern vector<const char*> roughnessMaps;
extern vector<const char*> normalMaps;
extern vector<const char*> aoMaps;

//���г����Ĳ�����׺����ǰ׺������ eg:cirus, door, factory, helipad 
extern vector<string> scenenames;

extern const string precomputepath;
extern string irradianceMapHDR;


//����ÿ��scenenames��cubemap·�� eg:"resources/textures/precompute/helipad/envCubeMap0.png"
//����Ǹ���scene�仯���仯����
extern vector<string> envCubemapFaces;
extern vector<string> IrradCubemapFaces;
extern vector<string> PrefilterCubemapFaces;

//����Ԥ����LUT��·��
extern const char* brdf_filepath;

//��hdr�����µ�textureID
extern unsigned int hdrTexture;
extern unsigned int irrad_hdrTexture;
extern unsigned int prefil_hdrTexture;
extern unsigned int ibllightposition;

//��cubemap��������µ� ����Ԥ���㲿�ֵ�textureID
extern unsigned int envCubemap;
extern unsigned int irradianceMap;
extern unsigned int prefilterMap;

//����LUT��textureID �����ڲ���
extern unsigned int brdfLUT;
extern unsigned int depthMap;
extern unsigned int frontdepthTexture;


//Camera����Ϣ ����position ����camera �����᷽������
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







