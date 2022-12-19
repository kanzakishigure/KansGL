#include "kspch.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>

#include "src/Render/Shader.h"
#include "src/Render/Camera.h"
#include "src/Render/Texture.h"
#include "src/Render/RenderBuffer.h"
#include "src/FileSystem/FileSystem.h"

#include "src/Core/Application.h"
#include "src/Utill/NativeModel.h"
namespace Kans
{

	static float ao = 1.0f;
	static glm::vec3 color = { 0.78,0.95,0.95 };
	static float metallic = 0.04f;
	static float roughness = 0.8f;
	int Main()
	{

		ApplicationSpecification spec;
		spec.Width = 1920;
		spec.Height = 1080;
		spec.renderfunc = nullptr;
		Application app = Application(spec);
		
		// opengl全局状态
		// -----------------------------
		glEnable(GL_DEPTH_TEST);
		//glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		//保证天空盒渲染不会遮挡物体
		glDepthFunc(GL_LEQUAL);

		//启用cubmap插值过滤,防止cubemap面与面之间产生走样
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

		// 编译shader
		// -------------------------
		
		//pbr材质shader
		Shader PBRshader(FileSystem::GetShaderPath("pbrShader.glsl"));
		PBRshader.Bind();
		PBRshader.SetFloat3("albedo", color);
		PBRshader.SetFloat("ao", ao);
		PBRshader.SetInt("irradianceMap", 0);
		PBRshader.SetInt("prefilterMap", 1);
		PBRshader.SetInt("brdfLUT", 2);
		
		
		
		
		Shader pbrTextureShader(FileSystem::GetShaderPath("pbrTextureShader.glsl"));
		//若是模型带有pbr材质贴图
		pbrTextureShader.Bind();
		pbrTextureShader.SetInt("irradianceMap", 0);
		pbrTextureShader.SetInt("prefilterMap", 1);
		pbrTextureShader.SetInt("brdfLUT", 2);

		pbrTextureShader.SetInt("albedoMap", 3);
		pbrTextureShader.SetInt("normalMap", 4);
		pbrTextureShader.SetInt("armMap", 5);


		//hdr环境贴图转换为CubeMap
		Shader hdrToCubeMapShader(FileSystem::GetShaderPath("HdrToCubeMap.glsl"));

		//天空盒
		Shader skyboxShader(FileSystem::GetShaderPath("skybox.glsl"));
		//分配texture采用插槽，后续同样操作不再赘述
		skyboxShader.SetInt("environmentMap", 0);

		//预计算 漫反射irradiance
		Shader prtShader(FileSystem::GetShaderPath("prtIrradiance.glsl"));
		prtShader.SetInt("environmentMap", 0);

		
		//预计算 镜面反射中的 Li(p,ωi)dωi 在半球上的积分
		// 本质的数学思想是使用球面谐波函数或者小波函数还原出半球上的光源分布，对于高频的信息只需要使用多个基函数就能表示
		//类似于预计算 irradiance，但是不同的点的粗糙程度不同，使用mipmap存储不同粗糙度的情况下的情况，既不同阶的基函数还原出的半球上的光源分布
		//越光滑的点，反射光线后的方向是大致相同的，使用高频信息来存储，此时的取mipmap上分辨率较高的层，就与之形成了对应。
		Shader prefilterShader(FileSystem::GetShaderPath("prefilter.glsl"));

		//预计算 镜面反射的brdf,用于查表
		Shader brdfShader(FileSystem::GetShaderPath("brdf.glsl"));



		//加载贴图

		Texture2D weathered_planks_diff = FileSystem::GetAssetPath("textures/pbr_texture/weathered_planks/weathered_planks_diff_2k.jpg");
		Texture2D weathered_planks_nor = FileSystem::GetAssetPath("textures/pbr_texture/weathered_planks/weathered_planks_nor_gl_2k.jpg");
		Texture2D weathered_planks_arm = FileSystem::GetAssetPath("textures/pbr_texture/weathered_planks/weathered_planks_arm_2k.png");










		// lights
		// ------
		glm::vec3 lightPositions[] = {
			glm::vec3(-15.0f,  15.0f, 15.0f),
			glm::vec3(15.0f,  15.0f, 15.0f),
			glm::vec3(-15.0f, -15.0f, 15.0f),
			glm::vec3(15.0f, -15.0f, 15.0f),
		};
		glm::vec3 lightColors[] = {
			glm::vec3(300.0f, 300.0f, 300.0f),
			glm::vec3(300.0f, 300.0f, 300.0f),
			glm::vec3(300.0f, 300.0f, 300.0f),
			glm::vec3(300.0f, 300.0f, 300.0f)
		};
		
		
		//使用hdr贴图作为环境光需要将hdr的柱状图展开为cubemap(非必须，但是shader使用的采样效率更高)
		//生成framebuffer，此处只需要存储colorbuffer不进行繁琐的存储类型，多个colorbuffer设置，固不将frambuffer抽象为一个类，减少代码量
		FrameBuffer framebuffer(512, 512);
		
		//加载hdr环境光贴图
		HDRTexture hdrTexture = FileSystem::GetAssetPath("textures/hdr/studio_country_hall_2k.hdr");
		

		

		// 设置cubemap6个面的viewmatrix
		// ----------------------------------------------------------------------------------------------
		glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		//从单位cube的中心点看向正方体6个面的view矩阵
		glm::mat4 captureViews[] =
		{
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		};
//-------将等柱状体的hdr贴图转换为cubemap---------------------------------------------------------------------------------------------------------------//
		
		hdrToCubeMapShader.Bind();
		hdrToCubeMapShader.SetInt("equirectangularMap", 0);
		hdrToCubeMapShader.SetMat4("projection", captureProjection);
		hdrTexture.Bind();
		// 我们转换的cubemap贴图大小为512 512，所以需要将视口调整
		glViewport(0, 0, 512, 512);
		
		//创建envCubemap用于存储转换的hdr贴图
		TextureSpec envCubemapspec;
		envCubemapspec.width = 512;
		envCubemapspec.height = 512;
		envCubemapspec.wrap = TextureWrap::CLAMP_TO_EDGE;
		envCubemapspec.minf = TextureFilter::LINEAR_MIPMAP_LINEAR;
		envCubemapspec.magf = TextureFilter::LINEAR;
		envCubemapspec.internalFormat = TextureInternalFormat::RGB16F;
		envCubemapspec.dataFormat = TextureFormat::RGB;
		TextureCube envCubemap(envCubemapspec);
		// 渲染填充cubemap
		framebuffer.Bind();
		for (unsigned int i = 0; i < 6; ++i)
		{
			hdrToCubeMapShader.SetMat4("view", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap.GetID(), 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			NativeModel::DrawCube();
		}
		framebuffer.UnBind();

		//使用opengl自动生成mipmap
		envCubemap.GenerateMipmap();
//------创建irradianceMap用于存储预计算的irradiance---------------------------------------------------------------------------------------------------//

		TextureCube irradiancemap(32, 32);
		
		framebuffer.ResizeRenderBuffer(32, 32);
		prtShader.Bind();
		prtShader.SetInt("environmentMap", 0);
		prtShader.SetMat4("projection", captureProjection);
		//使用填充后的envCubemap作为环境光，预计算irradiancemap
		envCubemap.Bind();
		
		//较低的分辨率已经可以满足卷积结果的保存，因为积分结果是相对平滑的，可以直接储存
		glViewport(0, 0, 32, 32);
		//渲染填充irradiancemap
		framebuffer.Bind();
		for (unsigned int i = 0; i < 6; ++i)
		{
			prtShader.SetMat4("view", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradiancemap.GetID(), 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			NativeModel::DrawCube();
		}
		framebuffer.UnBind();
//------ibl镜面反射项预滤波,生成mipmap---------------------------------------------------------------------------------------------------//
		TextureSpec prefilterMapspec;
		prefilterMapspec.width = 128;
		prefilterMapspec.height = 128;
		prefilterMapspec.wrap = TextureWrap::CLAMP_TO_EDGE;
		prefilterMapspec.minf = TextureFilter::LINEAR_MIPMAP_LINEAR;
		prefilterMapspec.magf = TextureFilter::LINEAR;
		prefilterMapspec.genMipmap = true;
		prefilterMapspec.internalFormat = TextureInternalFormat::RGB16F;
		prefilterMapspec.dataFormat = TextureFormat::RGB;
		TextureCube prefilterMap(prefilterMapspec);

		prefilterShader.Bind();
		prefilterShader.SetInt("environmentMap", 0);
		prefilterShader.SetMat4("projection", captureProjection);
		envCubemap.Bind();
		framebuffer.Bind();

		uint32_t  maxMipLevels = 5;
		for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
		{
			//调整 ViewPort大小,保证和 mip-level 维持统一
			unsigned int mipWidth = static_cast<unsigned int>(128 * std::pow(0.5, mip));
			unsigned int mipHeight = static_cast<unsigned int>(128 * std::pow(0.5, mip));
			framebuffer.ResizeRenderBuffer(mipWidth, mipHeight);
			glViewport(0, 0, mipWidth, mipHeight);

			float roughness = (float)mip / (float)(maxMipLevels - 1);
			prefilterShader.SetFloat("roughness", roughness);
			for (unsigned int i = 0; i < 6; ++i)
			{
				prefilterShader.SetMat4("view", captureViews[i]);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap.GetID(), mip);

				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				NativeModel::DrawCube();
			}
		}
		framebuffer.UnBind();

//------预计算2D LUT贴图,用于brdf查表---------------------------------------------------------------------------------------------------//

		TextureSpec  brdfLUTspec;
		brdfLUTspec.width = 512;
		brdfLUTspec.height = 512;
		brdfLUTspec.wrap = TextureWrap::CLAMP_TO_EDGE;
		brdfLUTspec.minf = TextureFilter::LINEAR;
		brdfLUTspec.magf = TextureFilter::LINEAR;
		brdfLUTspec.dataFormat = TextureFormat::RG;
		brdfLUTspec.internalFormat = TextureInternalFormat::RG16F;
		Texture2D brdfLUTTexture(brdfLUTspec);

		framebuffer.Bind();
		framebuffer.ResizeRenderBuffer(512, 512);
		framebuffer.SetColorAttachment(brdfLUTTexture.GetID());
		glViewport(0, 0, 512, 512);
		brdfShader.Bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		NativeModel::DrawQuad();
		framebuffer.UnBind();
		//恢复视口大小
		glViewport(0, 0, spec.Width, spec.Height);



		//渲染主循环函数
		auto renderFunction = [&](Camera& camera,int width,int height, TimeStep ts)
		{

			// render
			// ------
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// 渲染场景
			glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 100.0f);
			glm::mat4 view = camera.GetViewMatrix();
			

			
			{
				//生成材质球行列数
				int nrRows = 7;
				int nrColumns = 7;
				float spacing = 2.5;
				// 渲染材质球
				PBRshader.Bind();
				PBRshader.SetFloat3("albedo", color);
				PBRshader.SetFloat("ao", ao);

				PBRshader.SetMat4("projection", projection);
				PBRshader.SetMat4("view", view);
				PBRshader.SetFloat3("camPos", camera.Position);

				//使用ibl预计算项
				irradiancemap.Bind(0);
				prefilterMap.Bind(1);
				brdfLUTTexture.Bind(2);

				glm::mat4 model = glm::mat4(1.0f);

				model = glm::rotate(model, (float)glm::radians(90.0), glm::vec3(1.0, 0.0, 0.0));
				model = glm::scale(model, glm::vec3(20, 20, 1));
				model = glm::translate(model, glm::vec3(0.0, 0.0, 10.0));
				PBRshader.SetFloat("metallic", metallic);
				PBRshader.SetFloat("roughness", glm::clamp(roughness, 0.05f, 1.0f));
				PBRshader.SetMat4("model", model);
				NativeModel::DrawQuad();
				model = glm::mat4(1.0f);

				
				for (int row = 0; row < nrRows; ++row)
				{
					PBRshader.SetFloat("metallic", (float)row / (float)nrRows);
					for (int col = 0; col < nrColumns; ++col)
					{
						//粗糙度每一行增加
						PBRshader.SetFloat("roughness", glm::clamp((float)col / (float)nrColumns, 0.05f, 1.0f));

						model = glm::mat4(1.0f);
						model = glm::translate(model, glm::vec3(
							(col - (nrColumns / 2)) * spacing,
							(row - (nrRows / 2)) * spacing,
							0.0f
						));
						PBRshader.SetMat4("model", model);
						NativeModel::DrawSphere();
					}
				}



				PBRshader.Bind();
				//渲染代表光源的球体,未实现bloom，所以使用球体代替光源，本身无发光效果
				for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
				{
					glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * 5.0) * 5.0, 0.0, 0.0);
					newPos = lightPositions[i];
					PBRshader.SetFloat3("lightPositions[" + std::to_string(i) + "]", newPos);
					PBRshader.SetFloat3("lightColors[" + std::to_string(i) + "]", lightColors[i]);

					model = glm::mat4(1.0f);
					model = glm::translate(model, newPos);
					model = glm::scale(model, glm::vec3(0.5f));
					PBRshader.SetMat4("model", model);
					NativeModel::DrawSphere();
				}
			}


			{
				pbrTextureShader.Bind();
				glm::mat4 model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(0.0, -6, 5.0));
				model = glm::scale(model, glm::vec3(2.0, 2.0, 2.0));
				pbrTextureShader.SetMat4("model", model);

				pbrTextureShader.SetMat4("projection", projection);
				pbrTextureShader.SetMat4("view", view);
				pbrTextureShader.SetFloat3("camPos", camera.Position);

				pbrTextureShader.SetInt("irradianceMap", 0);
				pbrTextureShader.SetInt("prefilterMap", 1);
				pbrTextureShader.SetInt("brdfLUT", 2);

				pbrTextureShader.SetInt("albedoMap", 3);
				pbrTextureShader.SetInt("normalMap", 4);
				pbrTextureShader.SetInt("armMap", 5);

				irradiancemap.Bind(0);
				prefilterMap.Bind(1);
				brdfLUTTexture.Bind(2);
				weathered_planks_diff.Bind(3);
				weathered_planks_nor.Bind(4);
				weathered_planks_arm.Bind(5);
				
				NativeModel::DrawSphere();
			}
			//渲染天空盒
			{
				skyboxShader.Bind();
				skyboxShader.SetMat4("projection", projection);
				skyboxShader.SetMat4("view", view);
				envCubemap.Bind();
				NativeModel::DrawCube();
			}
			if (0)
			{
				glViewport(0, 0, 512, 512);
				brdfShader.Bind();
				NativeModel::DrawQuad();
			}
			
		};


		app.SetRenderFunction(renderFunction);

		auto imguifuc = [&](TimeStep ts) {

			ImGui::Begin("state");
			ImGui::Text("frame rate %f", 1.0 / ts);
			ImGui::Separator();
			ImGui::DragFloat("ao", &ao);
			ImGui::Separator();


			ImGui::SliderFloat("roughness", &roughness, 0, 1);
			ImGui::Separator();
			ImGui::SliderFloat("metallic", &metallic, 0, 1);
			ImGui::Separator();
			ImGui::ColorPicker3("color", glm::value_ptr(color));
			ImGui::End();
		};

		app.SetImguiRenderFunction(imguifuc);
		app.run();

		// 释放资源
		// ------------------------------------------------------------------------
		
		return 0;
	}

	
	

}

int main(int* argc, int** argv)
{
	return Kans::Main();
}