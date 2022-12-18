#include "kspch.h"
#include "Texture.h"
#include <stb_image.h>

namespace Kans
{

	Texture2D::Texture2D(const std::string& path)
		:path(path)
	{
		
		int width, height, channel;

		//保证与opengl中的uv坐标系吻合，翻转y轴
		stbi_set_flip_vertically_on_load(1);
		stbi_uc* data;
		{
			
			data = stbi_load(path.c_str(), &width, &height, &channel, 0);
			//文件路径，宽，高，通道，所需通道
			

		}


		m_Height = height;
		m_Width = width;

		switch (channel)
		{
		case 3: m_InternalFormat = GL_RGB8;  m_DataFormat = GL_RGB; break;
		case 4: m_InternalFormat = GL_RGBA8; m_DataFormat = GL_RGBA; break;
		}
		

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		//ID,贴图位置，x偏移量，y偏移量，宽，高，通道，数据类型，数据
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
		stbi_image_free(data);
	}

	Texture2D::Texture2D(uint32_t width, uint32_t height)
		:m_Width(width), m_Height(height)
	{
		
		m_InternalFormat = GL_RGBA8;
		m_DataFormat = GL_RGBA;

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
	}

	Texture2D::Texture2D(TextureSpec spec)
	{
		m_InternalFormat = (GLenum)spec.internalFormat;
		m_DataFormat = (GLenum)spec.dataFormat;
		m_Width = spec.width;
		m_Height = spec.height;

		glGenTextures(1, &m_RendererID);
		glBindTexture(GL_TEXTURE_2D, m_RendererID);


		glTexImage2D(GL_TEXTURE_2D, 0, m_InternalFormat, spec.width, spec.height, 0, m_DataFormat, GL_FLOAT, 0);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (GLuint)spec.wrap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (GLuint)spec.wrap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLuint)spec.minf);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLuint)spec.magf);

		if (spec.genMipmap)
		{
			glGenerateMipmap(GL_TEXTURE_2D);
		}
	}
	Texture2D::~Texture2D()
	{
		glDeleteTextures(1, &m_RendererID);
	}

	void Texture2D::Bind(uint32_t slot) const
	{

		glBindTextureUnit(slot, m_RendererID);
	}




	HDRTexture::HDRTexture(const std::string& path)
		:path(path)
	{
		int width, height, channel;

		//保证与opengl中的uv坐标系吻合，翻转y轴
		stbi_set_flip_vertically_on_load(1);
		float* data;
		{
			data = stbi_loadf(path.c_str(), &width, &height, &channel, 0);
			//文件路径，宽，高，通道，所需通道
		}


		m_Height = height;
		m_Width = width;

		switch (channel)
		{
		case 3: m_InternalFormat = GL_RGB16F;  m_DataFormat = GL_RGB; break;
		case 4: m_InternalFormat = GL_RGBA16F; m_DataFormat = GL_RGBA; break;
		}


		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//ID,贴图位置，x偏移量，y偏移量，宽，高，通道，数据类型，数据
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_FLOAT, data);
		stbi_image_free(data);
	}

	HDRTexture::HDRTexture(uint32_t width, uint32_t height)
		:m_Width(width), m_Height(height)
	{
		m_InternalFormat = GL_RGBA16F;
		m_DataFormat = GL_RGBA;

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
	}

	HDRTexture::~HDRTexture()
	{
		glDeleteTextures(1, &m_RendererID);
	}

	void HDRTexture::Bind(uint32_t slot /*= 0*/) const
	{
		glBindTextureUnit(slot, m_RendererID);
	}



	TextureCube::TextureCube(const std::string& path)
		:path(path)
	{
		
	}

	TextureCube::TextureCube(uint32_t width, uint32_t height)
		:m_Width(width), m_Height(height)
	{
		m_InternalFormat = GL_RGB16F;
		m_DataFormat = GL_RGB;
		glGenTextures(1, &m_RendererID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
		for (unsigned int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, m_InternalFormat, m_Width, m_Height, 0, m_DataFormat, GL_FLOAT, nullptr);
		}

		//贴图边缘处理设置
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		
		//贴图插值设置
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	TextureCube::TextureCube(TextureSpec spec)
	{
		m_Width = spec.width;
		m_Height = spec.height;
		m_InternalFormat = (GLenum)spec.internalFormat;
		m_DataFormat = (GLenum)spec.dataFormat;
		glGenTextures(1, &m_RendererID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);


		for (unsigned int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, m_InternalFormat, m_Width, m_Height, 0, m_DataFormat, GL_FLOAT, nullptr);
		}



		//贴图边缘处理设置
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, (GLuint)spec.wrap);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, (GLuint)spec.wrap);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, (GLuint)spec.wrap);

		//贴图插值设置
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, (GLuint)spec.minf);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, (GLuint)spec.magf);

		if (spec.genMipmap)
		{
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		}

	}
	TextureCube::~TextureCube()
	{
		glDeleteTextures(1, &m_RendererID);
	}

	void TextureCube::Bind(uint32_t slot /*= 0*/) const
	{
		glBindTextureUnit(slot, m_RendererID);
	}
	void TextureCube::GenerateMipmap()
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	}
}