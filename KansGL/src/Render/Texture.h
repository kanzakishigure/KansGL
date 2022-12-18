#pragma once
#include "Core/Base.h"
#include <glad/glad.h>

namespace Kans
{
	enum class TextureWrap
	{
		CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE
	};
	enum class TextureFilter
	{
		LINEAR_MIPMAP_LINEAR = GL_LINEAR_MIPMAP_LINEAR,
		LINEAR = GL_LINEAR,
		NEAREST = GL_NEAREST
	};
	enum class TextureFormat
	{
		RG = GL_RG,
		RGB = GL_RGB,
		RGBA = GL_RGBA
	};
	enum class TextureInternalFormat
	{
		RG16F = GL_RG16F,
		RGB16F = GL_RGB16F,
		RGBA16F = GL_RGBA16F,

		RG8 = GL_RG8,
		RGB8 = GL_RGB8,
		RGBA8 = GL_RGBA,
	};
	struct TextureSpec
	{
		uint32_t height;
		uint32_t width;
		bool genMipmap = false;
		TextureWrap wrap = TextureWrap::CLAMP_TO_EDGE;
		TextureFilter minf;
		TextureFilter magf;
		TextureFormat dataFormat = TextureFormat::RGB;
		TextureInternalFormat internalFormat = TextureInternalFormat::RGB16F;


	};
	class Texture2D
	{
	public:
		Texture2D(const std::string& path);
		Texture2D(uint32_t width, uint32_t height);
		Texture2D(TextureSpec spec);
		~Texture2D();
		void Bind(uint32_t slot = 0) const ;
		uint32_t GetID() { return m_RendererID; }
	private:
		uint32_t m_Height;
		uint32_t m_Width;
		uint32_t m_RendererID;
		GLenum m_InternalFormat, m_DataFormat;
		std::string path;
	};

	class HDRTexture
	{
	public:
		HDRTexture(const std::string& path);
		HDRTexture(uint32_t width, uint32_t height);
		~HDRTexture();
		void Bind(uint32_t slot = 0) const;
	private:
		uint32_t m_Height;
		uint32_t m_Width;
		uint32_t m_RendererID;
		GLenum m_InternalFormat, m_DataFormat;
		std::string path;
	};


	class TextureCube
	{
	public:
		TextureCube(const std::string& path);
		TextureCube(uint32_t width, uint32_t height);
		TextureCube(TextureSpec spec);
		~TextureCube();
		void Bind(uint32_t slot = 0) const;
		void GenerateMipmap();
		uint32_t GetID() { return m_RendererID; }
	private:
		uint32_t m_Height;
		uint32_t m_Width;
		uint32_t m_RendererID;
		GLenum m_InternalFormat, m_DataFormat;
		std::string path;
	};
}