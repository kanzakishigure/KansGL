#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>

typedef unsigned int GLenum;
namespace Kans
{
	class Shader 
	{
	public:
		Shader(const std::string& name, const std::string& VertexShaderpath, const std::string& FragmentShaderpath);
		Shader(const std::string& shaderpath);

		 ~Shader();

		 void Bind() const ;
		 void UnBind() const ;

		 const std::string& GetName() const  { return m_Name; }


		void SetMat4(const std::string& name, const glm::mat4& value) ;
		void SetFloat(const std::string& name, float value) ;
		void SetFloat2(const std::string& name, const glm::vec2& value) ;
		void SetFloat3(const std::string& name, const glm::vec3& value) ;
		void SetFloat4(const std::string& name, const glm::vec4& value) ;
		void SetInt(const std::string& name, int value) ;
		void SetInt2(const std::string& name, const glm::ivec2& value) ;
		void SetIntArray(const std::string& name, const int count, const int* value) ;
		void SetBool(const std::string& name, const bool value) ;
	public:
		uint32_t m_RendererID;
		std::string m_Name;
	private:

		std::string ReadFile(const std::string& filepath);
		std::unordered_map<GLenum, std::string> PreProcess(const std::string& source);
		void Compile(const std::unordered_map<GLenum, std::string>& shadersource);

	};
}

