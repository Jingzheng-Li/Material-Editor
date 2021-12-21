


#include <core/precompute.h>
#include <core/GL_utils.h>

#include <stb_image.h>
#include <stb_image_write.h>


using namespace std;


//===============================================================================================================

unsigned int CubemapPrecompute::loadTexturef(const char* path)
{

	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	stbi_set_flip_vertically_on_load(true);
	int width, height, nrComponents;
	float* data = stbi_loadf(path, &width, &height, &nrComponents, 0);


	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGB, GL_FLOAT, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	return textureID;
}

void CubemapPrecompute::renderCube()
{
	unsigned int cubeVAO = 0;
	unsigned int cubeVBO = 0;
	// initialize (if necessary)
	if (cubeVAO == 0)
	{
		float vertices[] = {
			// back face
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
			// front face
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			// left face
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			// right face
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
			// bottom face
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			// top face
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			 1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
			 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
		};
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// link vertex attributes
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// render Cube
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

void HDRPrecompute::renderQuad()
{
	unsigned int quadVAO = 0;
	unsigned int quadVBO = 0;
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

void HDRPrecompute::save_image(const char* filepath, int image_width, int image_height, int channels, const char* file_extension)
{
	const string& fileexternsion = file_extension;
	if (fileexternsion == "png")
	{
		GLsizei nrChannels = channels;
		GLsizei stride = nrChannels * image_width;
		stride += (stride % 4) ? (4 - stride % 4) : 0;
		GLsizei bufferSize = stride * image_height;
		vector<char> buffer(bufferSize);
		glPixelStorei(GL_PACK_ALIGNMENT, 4);
		glReadBuffer(GL_BACK);
		if (nrChannels == 4) glReadPixels(0, 0, image_width, image_height, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());
		else if (nrChannels == 3) glReadPixels(0, 0, image_width, image_height, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
		else if (nrChannels == 2) glReadPixels(0, 0, image_width, image_height, GL_RG, GL_UNSIGNED_BYTE, buffer.data());
		else if (nrChannels == 1) glReadPixels(0, 0, image_width, image_height, GL_RED, GL_UNSIGNED_BYTE, buffer.data());
		stbi_flip_vertically_on_write(true);
		stbi_write_png(filepath, image_width, image_height, nrChannels, buffer.data(), stride);
	}

	else if (fileexternsion == "hdr")
	{
		//图片带条纹
		GLsizei nrChannels = channels;
		GLsizei stride = nrChannels * image_width;
		stride += (stride % 4) ? (4 - stride % 4) : 0;
		GLsizei bufferSize = stride * image_height;
		vector<float> buffer(bufferSize);
		glPixelStorei(GL_PACK_ALIGNMENT, 4);
		glReadBuffer(GL_BACK);
		glReadPixels(0, 0, image_width, image_height, GL_RGBA, GL_FLOAT, buffer.data());
		stbi_flip_vertically_on_write(true);
		stbi_write_hdr(filepath, image_width, image_height, nrChannels, buffer.data());

	}
	else cout << "error with image file_extension of ." << file_extension << endl;

}

//===============================================================================================================





//===============================================================================================================
void Precompute::PrecomputeProcess(const char* hdr_filepath)
{

	// pbr: setup framebuffer
	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 4096, 2048);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);


	//首先定义envCubemap作为texture pbr: setup cubemap to render to and attach to framebuffer
	glGenTextures(1, &envCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	// enable pre-filter mipmap sampling (combatting visible dots artifact)
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	hdrTexture = loadTexturef(hdr_filepath);

	//生成所有的cubemap形式的预计算
	envCubemap = equirectangular_to_cubemap(equirectangularToCubemapShader, hdrTexture);
	irradianceMap = PrecomputeIrradianceMap(irradianceShader, irradianceMap);
	//分层生成所有的cubemap prefilterMap
	prefilterMap0 = PrecomputePrefilterMap(prefilterShader, prefilterMap0, 0);
	prefilterMap1 = PrecomputePrefilterMap(prefilterShader, prefilterMap1, 1);
	prefilterMap2 = PrecomputePrefilterMap(prefilterShader, prefilterMap2, 2);
	prefilterMap3 = PrecomputePrefilterMap(prefilterShader, prefilterMap3, 3);
	prefilterMap4 = PrecomputePrefilterMap(prefilterShader, prefilterMap4, 4);


}

void Precompute::PrecomputeProcessHDR(const char* irradiancepath, const char* prefilterpackpath)
{

	// pbr: setup framebuffer
	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 4096, 2048);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

	//预计算irradianceMap
	cubemap_to_equirectangular(irradianceMap, 2048, 1024, irradiancepath);

	//每层prefilter生成textureID
	unsigned int mipWidth = 2048, mipHeight = 1024;
	hdrprefilterMap0 = cubemap_to_equirectangular(prefilterMap0, mipWidth, mipHeight);
	mipWidth *= 0.5, mipHeight *= 0.5;
	hdrprefilterMap1 = cubemap_to_equirectangular(prefilterMap1, mipWidth, mipHeight);
	mipWidth *= 0.5, mipHeight *= 0.5;
	hdrprefilterMap2 = cubemap_to_equirectangular(prefilterMap2, mipWidth, mipHeight);
	mipWidth *= 0.5, mipHeight *= 0.5;
	hdrprefilterMap3 = cubemap_to_equirectangular(prefilterMap3, mipWidth, mipHeight);
	mipWidth *= 0.5, mipHeight *= 0.5;
	hdrprefilterMap4 = cubemap_to_equirectangular(prefilterMap4, mipWidth, mipHeight);

	PrecomputeProcessPackHDR(prefilterpackpath);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

}

void Precompute::PrecomputeProcessLUT(const char* brdffilepath, const char* IBLLightPosition)
{
	// pbr: setup framebuffer
	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 2048, 2048);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

	//寻找IBL的光源方向
	Precompute_IBLLightPosition(hdrLightShader, IBLLightPosition);
	//计算IBl贴图
	unsigned char* data = stbi_load(brdffilepath, &TextureWidth, &TextureHeight, &TextureChannels, 0);
	if (!data) Precompute_brdfLUTTexture(brdfShader, brdffilepath);
	
}

//===============================================================================================================



//===============================================================================================================
unsigned int CubemapPrecompute::equirectangular_to_cubemap(Shader ourshader, unsigned int hdrTexture)
{

	// pbr: convert HDR equirectangular environment map to cubemap equivalent
	// ----------------------------------------------------------------------
	ourshader.use();
	ourshader.setInt("equirectangularMap", 0);
	ourshader.setMat4("projection", captureProjection);


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdrTexture);

	glViewport(0, 0, 512, 512); // don't forget to configure the viewport to the capture dimensions.
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	for (unsigned int i = 0; i < 6; ++i)
	{
		ourshader.setMat4("view", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		renderCube();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
	// generate mipmaps from first mip face (combatting visible dots artifact)
	// 	这一块主要是为了处理在粗糙的mip级别上可能会出现明亮区域周围出现点状图案
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return envCubemap;
}

unsigned int CubemapPrecompute::PrecomputeIrradianceMap(Shader ourshader, unsigned int irradianceMap)
{

	glGenTextures(1, &irradianceMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 256, 256, 0, GL_RGB, GL_FLOAT, NULL);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 256, 256);

	// pbr: solve diffuse integral by convolution to create an irradiance (cube)map.
	// -----------------------------------------------------------------------------
	ourshader.use();
	ourshader.setInt("environmentMap", 0);
	ourshader.setMat4("projection", captureProjection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

	glViewport(0, 0, 256, 256); // don't forget to configure the viewport to the capture dimensions.
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	for (unsigned int i = 0; i < 6; ++i)
	{
		ourshader.setMat4("view", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		renderCube();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return irradianceMap;
}

unsigned int CubemapPrecompute::PrecomputePrefilterMap(Shader ourshader, unsigned int prefilterMap, unsigned int mip)
{
	unsigned int mipWidth = 512 * pow(0.5, mip);
	unsigned int mipHeight = 512 * pow(0.5, mip);

	glGenTextures(1, &prefilterMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, mipWidth, mipHeight, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	//这里只是给framebuffer留足够的空间 实际上不开后面generateMipmap只是度空间而已
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	ourshader.use();
	ourshader.setInt("environmentMap", 0);
	ourshader.setMat4("projection", captureProjection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

	//只需要把roughness给好 就可以了
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
	glViewport(0, 0, mipWidth, mipHeight);
	float roughness = (float)mip / (float)(maxMipLevels - 1);
	ourshader.setFloat("roughness", roughness);

	for (unsigned int i = 0; i < 6; ++i)
	{
		ourshader.setMat4("view", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		renderCube();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return prefilterMap;

}
//===============================================================================================================




//===============================================================================================================
unsigned int HDRPrecompute::cubemap_to_equirectangular(unsigned int PrecomputeMap, unsigned int width, unsigned int height, const char* hdrsavepath)
{

	unsigned int CUBEMAP_TO_HDR_TEXTURE;
	glGenTextures(1, &CUBEMAP_TO_HDR_TEXTURE);
	glBindTexture(GL_TEXTURE_2D, CUBEMAP_TO_HDR_TEXTURE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	CubemapToequirectangularShader.use();
	CubemapToequirectangularShader.setInt("environmentMap", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, PrecomputeMap);

	glViewport(0, 0, width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, CUBEMAP_TO_HDR_TEXTURE, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderQuad();

	save_image(hdrsavepath, width, height, 4, "hdr");

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	return CUBEMAP_TO_HDR_TEXTURE;

}

unsigned int HDRPrecompute::cubemap_to_equirectangular(unsigned int PrecomputeMap, unsigned int width, unsigned int height)
{

	unsigned int CUBEMAP_TO_HDR_TEXTURE;
	glGenTextures(1, &CUBEMAP_TO_HDR_TEXTURE);
	glBindTexture(GL_TEXTURE_2D, CUBEMAP_TO_HDR_TEXTURE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	CubemapToequirectangularShader.use();
	CubemapToequirectangularShader.setInt("environmentMap", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, PrecomputeMap);


	glViewport(0, 0, width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, CUBEMAP_TO_HDR_TEXTURE, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderQuad();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	return CUBEMAP_TO_HDR_TEXTURE;

}

void HDRPrecompute::PrecomputeProcessPackHDR(const char* prefilterpackpath)
{
	//首先为prefilterpack分配一个足够大的空间
	unsigned int PACK_HDR_TEXTURE;
	glGenTextures(1, &PACK_HDR_TEXTURE);
	glBindTexture(GL_TEXTURE_2D, PACK_HDR_TEXTURE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 2048, 1536, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 2048, 1536);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, PACK_HDR_TEXTURE, 0);


	float* data = new float[2048 * 1024 * 4];
	PrefilterPackArrangement(data, hdrprefilterMap0, PACK_HDR_TEXTURE, 2048, 1024, 0, 0);
	PrefilterPackArrangement(data, hdrprefilterMap1, PACK_HDR_TEXTURE, 1024, 512, 0, 1024);
	PrefilterPackArrangement(data, hdrprefilterMap2, PACK_HDR_TEXTURE, 512, 256, 1024, 1024);
	PrefilterPackArrangement(data, hdrprefilterMap3, PACK_HDR_TEXTURE, 256, 128, 1536, 1024);
	PrefilterPackArrangement(data, hdrprefilterMap4, PACK_HDR_TEXTURE, 128, 64, 1792, 1024);


	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 2048, 1536, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//保存pack图
	hdrTextureShader.use();
	hdrTextureShader.setInt("environmentMap", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, PACK_HDR_TEXTURE);

	glViewport(0, 0, 2048, 1536);
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderQuad();
	save_image(prefilterpackpath, 2048, 1536, 4, "hdr");
	
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void HDRPrecompute::PrefilterPackArrangement(float* data, unsigned int hdrprefilterMap, unsigned int PACK_HDR_TEXTURE, int Texwidth, int Texheight, int PosX, int PosY)
{
	glBindTexture(GL_TEXTURE_2D, hdrprefilterMap);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, data);
	glBindTexture(GL_TEXTURE_2D, PACK_HDR_TEXTURE);
	glTexSubImage2D(GL_TEXTURE_2D, 0, PosX, PosY, Texwidth, Texheight, GL_RGB, GL_FLOAT, data);
	glBindTexture(GL_TEXTURE_2D, 0);
	memset(data, '\0', sizeof(data) * 1);
}
//===============================================================================================================





//===========================================================================================================================
unsigned int HDRPrecompute::Precompute_brdfLUTTexture(Shader ourshader, const char* filename)
{
	
	// pre-allocate enough memory for the LUT texture.
	glGenTextures(1, &brdfLUT);
	glBindTexture(GL_TEXTURE_2D, brdfLUT);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, 0);
	// be sure to set wrapping mode to GL_CLAMP_TO_EDGE
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// then re-configure capture framebuffer object and render screen-space quad with BRDF shader.
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUT, 0);

	glViewport(0, 0, 512, 512);
	ourshader.use();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderQuad();

	save_image(filename, 512, 512, 3, "png");

	// keep all rendering operation can be viewed on the screen, to active frambuffer again
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return brdfLUT;

}

unsigned int HDRPrecompute::Precompute_IBLLightPosition(Shader ourshader, const char* filename)
{	

	//先生成一个texture用来保存图片
	glGenTextures(1, &lightpositionMap);
	glBindTexture(GL_TEXTURE_2D, lightpositionMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 2048, 1024, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	ourshader.use();
	ourshader.setInt("environmentMap", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdrprefilterMap1);

	glViewport(0, 0, 2048, 1024);
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lightpositionMap, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderQuad();

	save_image(filename, 2048, 1024, 4, "hdr");

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	return lightpositionMap;

}

//===========================================================================================================================
