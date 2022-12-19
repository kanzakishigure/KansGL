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
		
		// openglȫ��״̬
		// -----------------------------
		glEnable(GL_DEPTH_TEST);
		//glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		//��֤��պ���Ⱦ�����ڵ�����
		glDepthFunc(GL_LEQUAL);

		//����cubmap��ֵ����,��ֹcubemap������֮���������
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

		// ����shader
		// -------------------------
		
		//pbr����shader
		Shader PBRshader(FileSystem::GetShaderPath("pbrShader.glsl"));
		PBRshader.Bind();
		PBRshader.SetFloat3("albedo", color);
		PBRshader.SetFloat("ao", ao);
		PBRshader.SetInt("irradianceMap", 0);
		PBRshader.SetInt("prefilterMap", 1);
		PBRshader.SetInt("brdfLUT", 2);
		
		
		
		
		Shader pbrTextureShader(FileSystem::GetShaderPath("pbrTextureShader.glsl"));
		//����ģ�ʹ���pbr������ͼ
		pbrTextureShader.Bind();
		pbrTextureShader.SetInt("irradianceMap", 0);
		pbrTextureShader.SetInt("prefilterMap", 1);
		pbrTextureShader.SetInt("brdfLUT", 2);

		pbrTextureShader.SetInt("albedoMap", 3);
		pbrTextureShader.SetInt("normalMap", 4);
		pbrTextureShader.SetInt("armMap", 5);


		//hdr������ͼת��ΪCubeMap
		Shader hdrToCubeMapShader(FileSystem::GetShaderPath("HdrToCubeMap.glsl"));

		//��պ�
		Shader skyboxShader(FileSystem::GetShaderPath("skybox.glsl"));
		//����texture���ò�ۣ�����ͬ����������׸��
		skyboxShader.SetInt("environmentMap", 0);

		//Ԥ���� ������irradiance
		Shader prtShader(FileSystem::GetShaderPath("prtIrradiance.glsl"));
		prtShader.SetInt("environmentMap", 0);

		
		//Ԥ���� ���淴���е� Li(p,��i)d��i �ڰ����ϵĻ���
		// ���ʵ���ѧ˼����ʹ������г����������С��������ԭ�������ϵĹ�Դ�ֲ������ڸ�Ƶ����Ϣֻ��Ҫʹ�ö�����������ܱ�ʾ
		//������Ԥ���� irradiance�����ǲ�ͬ�ĵ�Ĵֲڳ̶Ȳ�ͬ��ʹ��mipmap�洢��ͬ�ֲڶȵ�����µ�������Ȳ�ͬ�׵Ļ�������ԭ���İ����ϵĹ�Դ�ֲ�
		//Խ�⻬�ĵ㣬������ߺ�ķ����Ǵ�����ͬ�ģ�ʹ�ø�Ƶ��Ϣ���洢����ʱ��ȡmipmap�Ϸֱ��ʽϸߵĲ㣬����֮�γ��˶�Ӧ��
		Shader prefilterShader(FileSystem::GetShaderPath("prefilter.glsl"));

		//Ԥ���� ���淴���brdf,���ڲ��
		Shader brdfShader(FileSystem::GetShaderPath("brdf.glsl"));



		//������ͼ

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
		
		
		//ʹ��hdr��ͼ��Ϊ��������Ҫ��hdr����״ͼչ��Ϊcubemap(�Ǳ��룬����shaderʹ�õĲ���Ч�ʸ���)
		//����framebuffer���˴�ֻ��Ҫ�洢colorbuffer�����з����Ĵ洢���ͣ����colorbuffer���ã��̲���frambuffer����Ϊһ���࣬���ٴ�����
		FrameBuffer framebuffer(512, 512);
		
		//����hdr��������ͼ
		HDRTexture hdrTexture = FileSystem::GetAssetPath("textures/hdr/studio_country_hall_2k.hdr");
		

		

		// ����cubemap6�����viewmatrix
		// ----------------------------------------------------------------------------------------------
		glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		//�ӵ�λcube�����ĵ㿴��������6�����view����
		glm::mat4 captureViews[] =
		{
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		};
//-------������״���hdr��ͼת��Ϊcubemap---------------------------------------------------------------------------------------------------------------//
		
		hdrToCubeMapShader.Bind();
		hdrToCubeMapShader.SetInt("equirectangularMap", 0);
		hdrToCubeMapShader.SetMat4("projection", captureProjection);
		hdrTexture.Bind();
		// ����ת����cubemap��ͼ��СΪ512 512��������Ҫ���ӿڵ���
		glViewport(0, 0, 512, 512);
		
		//����envCubemap���ڴ洢ת����hdr��ͼ
		TextureSpec envCubemapspec;
		envCubemapspec.width = 512;
		envCubemapspec.height = 512;
		envCubemapspec.wrap = TextureWrap::CLAMP_TO_EDGE;
		envCubemapspec.minf = TextureFilter::LINEAR_MIPMAP_LINEAR;
		envCubemapspec.magf = TextureFilter::LINEAR;
		envCubemapspec.internalFormat = TextureInternalFormat::RGB16F;
		envCubemapspec.dataFormat = TextureFormat::RGB;
		TextureCube envCubemap(envCubemapspec);
		// ��Ⱦ���cubemap
		framebuffer.Bind();
		for (unsigned int i = 0; i < 6; ++i)
		{
			hdrToCubeMapShader.SetMat4("view", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap.GetID(), 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			NativeModel::DrawCube();
		}
		framebuffer.UnBind();

		//ʹ��opengl�Զ�����mipmap
		envCubemap.GenerateMipmap();
//------����irradianceMap���ڴ洢Ԥ�����irradiance---------------------------------------------------------------------------------------------------//

		TextureCube irradiancemap(32, 32);
		
		framebuffer.ResizeRenderBuffer(32, 32);
		prtShader.Bind();
		prtShader.SetInt("environmentMap", 0);
		prtShader.SetMat4("projection", captureProjection);
		//ʹ�������envCubemap��Ϊ�����⣬Ԥ����irradiancemap
		envCubemap.Bind();
		
		//�ϵ͵ķֱ����Ѿ���������������ı��棬��Ϊ���ֽ�������ƽ���ģ�����ֱ�Ӵ���
		glViewport(0, 0, 32, 32);
		//��Ⱦ���irradiancemap
		framebuffer.Bind();
		for (unsigned int i = 0; i < 6; ++i)
		{
			prtShader.SetMat4("view", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradiancemap.GetID(), 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			NativeModel::DrawCube();
		}
		framebuffer.UnBind();
//------ibl���淴����Ԥ�˲�,����mipmap---------------------------------------------------------------------------------------------------//
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
			//���� ViewPort��С,��֤�� mip-level ά��ͳһ
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

//------Ԥ����2D LUT��ͼ,����brdf���---------------------------------------------------------------------------------------------------//

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
		//�ָ��ӿڴ�С
		glViewport(0, 0, spec.Width, spec.Height);



		//��Ⱦ��ѭ������
		auto renderFunction = [&](Camera& camera,int width,int height, TimeStep ts)
		{

			// render
			// ------
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// ��Ⱦ����
			glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 100.0f);
			glm::mat4 view = camera.GetViewMatrix();
			

			
			{
				//���ɲ�����������
				int nrRows = 7;
				int nrColumns = 7;
				float spacing = 2.5;
				// ��Ⱦ������
				PBRshader.Bind();
				PBRshader.SetFloat3("albedo", color);
				PBRshader.SetFloat("ao", ao);

				PBRshader.SetMat4("projection", projection);
				PBRshader.SetMat4("view", view);
				PBRshader.SetFloat3("camPos", camera.Position);

				//ʹ��iblԤ������
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
						//�ֲڶ�ÿһ������
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
				//��Ⱦ�����Դ������,δʵ��bloom������ʹ����������Դ�������޷���Ч��
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
			//��Ⱦ��պ�
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

		// �ͷ���Դ
		// ------------------------------------------------------------------------
		
		return 0;
	}

	
	

}

int main(int* argc, int** argv)
{
	return Kans::Main();
}