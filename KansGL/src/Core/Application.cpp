#include "kspch.h"
#include "Application.h"


#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <glad/glad.h>




#define ImguiON true
namespace Kans
{
	Kans::Application* Application::s_Instance;


	// ----------------------------------------------Utill Function---------------------------------------------------------//
	void framebuffer_size_callback(GLFWwindow* window, int width, int height)
	{
		//更新frameBuffer的大小，保持与窗口大小一致
		glViewport(0, 0, width, height);
	}
	//鼠标事件回调函数
	void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
	{
		CameraData& data = *(CameraData*)glfwGetWindowUserPointer(window);
		
		
		if (data.keycode== GLFW_MOUSE_BUTTON_RIGHT)
		{
			float xpos = static_cast<float>(xposIn);
			float ypos = static_cast<float>(yposIn);
			if (data.firstMouse)
			{
				data.lastX = xpos;
				data.lastY = ypos;
				data.firstMouse = false;
			}

			float xoffset = xpos - data.lastX;
			float yoffset = data.lastY - ypos; // reversed since y-coordinates go from bottom to top

			data.lastX = xpos;
			data.lastY = ypos;

			data.pCamera->ProcessMouseMovement(xoffset, yoffset);
		}
	}
	//鼠标滚轮回调函数
	void scroll_callback(GLFWwindow* window,double xoffset, double yoffset)
	{
		CameraData& data = *(CameraData*)glfwGetWindowUserPointer(window);
		data.pCamera->ProcessMouseScroll(static_cast<float>(yoffset));
	}

	// ----------------------------------------------Application Function---------------------------------------------------------//
	Application::Application(ApplicationSpecification spec)
		:m_RunderFunction(spec.renderfunc),m_Width(spec.Width),m_Height(spec.Height)
	{
		m_Data.pCamera = &m_Camera;
		m_Data.lastX = spec.Width / 2.0;
		m_Data.lastY = spec.Height / 2.0;


		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		m_Window= glfwCreateWindow(spec.Width, spec.Height, "KansGL", NULL, NULL);
		if (m_Window == NULL)
		{
			std::cout << "Failed to create GLFW window" << std::endl;
			glfwTerminate();
			return ;
		}

		glfwSetWindowUserPointer(m_Window, &m_Data);

		glfwMakeContextCurrent(m_Window);
		glfwSetFramebufferSizeCallback(m_Window, framebuffer_size_callback);
		glfwSetCursorPosCallback(m_Window, mouse_callback);
		glfwSetScrollCallback(m_Window, scroll_callback);

		// GLFW捕获鼠标
		//glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


		// 使用Glad加载具体驱动函数
		// ---------------------------------------
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			std::cout << "Failed to initialize GLAD" << std::endl;
			return ;
		}


		InitImgui();
	}

	Application::~Application()
	{
		glfwTerminate();
		ShutDownImgui();
	}

	void Application::InitImgui()
	{
#if ImguiON 
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;

		//Kans:
		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsClassic();
		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}


		// Setup Platform/Renderer bindings
		ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
		//ImGui_ImplGlfw_InitForVulkan(window, true);
		ImGui_ImplOpenGL3_Init("#version 410");
#endif
	}

	void Application::ShutDownImgui()
	{
#if ImguiON
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
#endif
	}

	void Application::ImguiBegin()
	{
#if ImguiON
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
#endif
	}

	void Application::ImguiEnd()
	{
#if ImguiON
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(m_Width,m_Height);

		// Rendering
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
#endif
	}

	void Application::OnImguiRender()
	{
#if ImguiON
		m_ImguiFunction(m_Frametime);
#endif
	}

	void Application::run()
	{
		while (!glfwWindowShouldClose(m_Window))
		{
			float time = GetTime();
			m_Frametime = TimeStep(float(glfwGetTime() - m_LastFrameTime));
			m_TimeStep = glm::min<float>(m_Frametime, 0.0333f);
			m_LastFrameTime = time;

			//渲染场景
			{
				m_RunderFunction(m_Camera, m_Width, m_Height, m_Frametime);
			}
			
			//渲染UI
			{
				ImguiBegin();
				OnImguiRender();
				ImguiEnd();
			}

			glfwSwapBuffers(m_Window);
			glfwPollEvents();
			Update();
			
		}
	}

	void Application::Update()
	{
		m_Data.keycode = 0;
		if (glfwGetKey(m_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(m_Window, true);

		if (glfwGetKey(m_Window, GLFW_KEY_W) == GLFW_PRESS)
			m_Camera.ProcessKeyboard(FORWARD, m_Frametime);
		if (glfwGetKey(m_Window, GLFW_KEY_S) == GLFW_PRESS)
			m_Camera.ProcessKeyboard(BACKWARD, m_Frametime);
		if (glfwGetKey(m_Window, GLFW_KEY_A) == GLFW_PRESS)
			m_Camera.ProcessKeyboard(LEFT, m_Frametime);
		if (glfwGetKey(m_Window, GLFW_KEY_D) == GLFW_PRESS)
			m_Camera.ProcessKeyboard(RIGHT, m_Frametime);

		if (glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
		{
			
			m_Data.keycode = GLFW_MOUSE_BUTTON_RIGHT;
		}
	}

	float Application::GetTime() const
	{
		return (float)glfwGetTime();
	}
	

}