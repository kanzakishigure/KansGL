#pragma once
#include "TimeStep.h"
#include <functional>
#include "Render/Camera.h"
struct  GLFWwindow;
namespace Kans
{
	
	struct ApplicationSpecification
	{
		int Width;
		int Height;
		std::function<void(Camera&, int, int, TimeStep)> renderfunc;
		
	};
	struct CameraData
	{
		float lastX = 0.0;;
		float lastY = 0.0;
		bool firstMouse = true;
		Camera* pCamera = nullptr;
		uint32_t keycode;
	};
	class  Application
	{
	public:
		Application(ApplicationSpecification spec);

		//��֤application�����������ʱ���ܵ��õ������������������
		virtual ~Application();

		void InitImgui();
		void ShutDownImgui();
		void ImguiBegin();
		void ImguiEnd();
		void OnImguiRender();

		void SetRenderFunction(std::function<void(Camera,int,int,TimeStep)> func) { m_RunderFunction = func; }
		void SetImguiRenderFunction(std::function<void(TimeStep)> func) { m_ImguiFunction = func; }
		void run();
		void Update();
		//���ص���application�ĵ��������Բ�Ӧ�ý�ָ�뷵��
		inline static Application& Get() { return *s_Instance; }
		const Camera& GetCamera() { return m_Camera; }


		float GetTime() const;
		TimeStep GetTimestep() const { return m_TimeStep; }
		TimeStep GetFrametime() const { return m_Frametime; }


	private:
		GLFWwindow* m_Window;
		static Application* s_Instance;
		
		std::function<void(Camera&, int, int, TimeStep)> m_RunderFunction;
		std::function<void(TimeStep)> m_ImguiFunction;

		int m_Width;
		int m_Height;

		TimeStep m_TimeStep;
		TimeStep m_Frametime;
		float m_LastFrameTime = 0.0f;

		
		//�������
		Camera m_Camera = glm::vec3(0.0f, 0.0f, 40.0f);
		CameraData m_Data;
		

	};
}