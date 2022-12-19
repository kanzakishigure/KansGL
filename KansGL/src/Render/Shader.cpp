#include "kspch.h"
#include "Shader.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Kans
{
	static GLenum ShaderTypeFromString(const std::string& type)
	{
		if (type == "vertex")
			return GL_VERTEX_SHADER;
		if (type == "fragment")
			return GL_FRAGMENT_SHADER;
		if (type == "geometry")
			return GL_GEOMETRY_SHADER;
		return 0;
	}

	Shader::Shader(const std::string& name, const std::string& VertexShaderpath, const std::string& FragmentShaderpath)
	{
		std::unordered_map<GLenum, std::string> shadersource;
		std::string t = ReadFile(VertexShaderpath);
		shadersource[GL_VERTEX_SHADER] = ReadFile(VertexShaderpath);
		shadersource[GL_FRAGMENT_SHADER] = ReadFile(FragmentShaderpath);
		Compile(shadersource);
		m_Name = name;
	}

	Shader::Shader(const std::string& shaderpath)
	{
		std::string shadersrc = ReadFile(shaderpath);
		auto source = PreProcess(shadersrc);
		Compile(source);
		//从文件名获取shader的命名
		auto lastSlash = shaderpath.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = shaderpath.rfind('.');
		auto count = lastDot == std::string::npos ? shaderpath.size() - lastSlash : lastDot - lastSlash;
		m_Name = shaderpath.substr(lastSlash, count);
	}

	Shader::~Shader()
	{
		glDeleteProgram(m_RendererID);
	}

	void Shader::Bind() const
	{
		glUseProgram(m_RendererID);
	}

	void Shader::UnBind() const
	{
		glUseProgram(0);
	}
	void Shader::SetMat4(const std::string& name, const glm::mat4& value)
	{
		
		uint32_t location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
	}

	void Shader::SetFloat(const std::string& name, float value)
	{
		
		uint32_t location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1f(location, value);
	}

	void Shader::SetFloat2(const std::string& name, const glm::vec2& value)
	{
		
		uint32_t location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform2f(location, value.x, value.y);
	}

	void Shader::SetFloat3(const std::string& name, const glm::vec3& value)
	{
		
		uint32_t location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform3f(location, value.x, value.y, value.z);
	}

	void Shader::SetFloat4(const std::string& name, const glm::vec4& value)
	{
		uint32_t location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform4fv(location, 1, glm::value_ptr(value));
	}

	void Shader::SetInt(const std::string& name, int value)
	{
		
		uint32_t location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1i(location, value);
	}

	void Shader::SetInt2(const std::string& name, const glm::ivec2& value)
	{
		
		uint32_t location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform2i(location, value.x, value.y);
	}

	void Shader::SetIntArray(const std::string& name, const int count, const int* value)
	{
		
		uint32_t location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1iv(location, count, value);
	}

	void Shader::SetBool(const std::string& name, const bool value)
	{
		
		uint32_t location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1i(location, value);
	}

	std::string Shader::ReadFile(const std::string& filepath)
	{
		std::string result;
		std::ifstream in(filepath, std::ios::in | std::ios::binary);
		if (in)
		{
			//为当前文件流设置下一个读取的流位置
			/*
			ios_base::beg	beginning of the stream
			ios_base::cur	current position in the stream
			ios_base::end	end of the stream
			*/
			//偏移量，类型
			in.seekg(0, std::ios::end);
			//将tellg移动到文件尾
			result.resize(in.tellg());
			//tellg() 
			//Returns the position of the current character in the input stream.
			in.seekg(0, std::ios::beg);

			in.read(&result[0], result.size());
			in.close();
		}

		return result;
	}

	std::unordered_map<GLenum, std::string> Shader::PreProcess(const std::string& source)
	{
		const char* typeToken = "#type";
		size_t typeTokenlength = strlen(typeToken);
		size_t pos = source.find_first_of(typeToken);
		std::unordered_map<GLenum, std::string> shadersource;
		while (pos != std::string::npos)
		{
			size_t eol = source.find_first_of("\n\r", pos);
			size_t begin = pos + typeTokenlength + 1;
			std::string type = source.substr(begin, eol - begin);
			size_t nextlinepos = source.find_first_not_of("\r\n", eol);
			pos = source.find(typeToken, nextlinepos);
			shadersource[ShaderTypeFromString(type)] = source.substr(
				nextlinepos, pos - (nextlinepos == std::string::npos ? source.size() - 1 : nextlinepos)
			);

		}
		return shadersource;
	}

	void Shader::Compile(const std::unordered_map<GLenum, std::string>& shadersource)
	{
		//shaderComplie
		uint32_t id = glCreateProgram();
		std::array<GLenum, 3> glShaderIDs;
		uint32_t index = 0;
		GLenum test1 = GL_FRAGMENT_SHADER;
		GLenum test2 = GL_VERTEX_SHADER;
		int test3 = GL_GEOMETRY_SHADER;
		for each (auto & kv in shadersource)
		{
			GLenum type = kv.first;
			std::string source = kv.second;
			uint32_t shader = glCreateShader(type);
			GLint isCompiled = 0;
			const GLchar* sourceStr = source.c_str();
			glShaderSource(shader, 1, &sourceStr, nullptr);
			glCompileShader(shader);
			//查看是否编译成功
			glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
				std::vector<GLchar> infolog(maxLength);
				glGetShaderInfoLog(shader, maxLength, &maxLength, &infolog[0]);

				std::cout << infolog.data()<<std::endl;
				//删除shader文件
				glDeleteShader(shader);
				break;
			}
			glAttachShader(id, shader);
			glShaderIDs[index++] = shader;
		}


		//shaderProgram
		GLint isLinked = 0;

		glLinkProgram(id);
		glGetProgramiv(id, GL_LINK_STATUS, (int*)&isLinked);

		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(id, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infolog(maxLength);
			glGetProgramInfoLog(id, maxLength, &maxLength, &infolog[0]);

			std::cout << infolog.data() << std::endl;
			//防止内存泄漏，删除程序
			glDeleteProgram(id);
			// 同理.
			for each (auto shaderid in glShaderIDs)
			{
				glDeleteShader(shaderid);
			}
			return;
		}
		//成功链接之后不用继续链接
		for each (auto shaderid in glShaderIDs)
		{
			glDetachShader(id, shaderid);
			glDeleteShader(shaderid);
		}
		m_RendererID = id;
	}

}