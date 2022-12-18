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

		//保证application在派生类调用时，能调用到派生类的虚析构函数
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
		//返回的是application的单例，所以不应该将指针返回
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

		
		//场景相机
		Camera m_Camera = glm::vec3(0.0f, 0.0f, 40.0f);
		CameraData m_Data;
		

	};
}