#include "kspch.h"
#include "RenderBuffer.h"

#include <glad/glad.h>
namespace Kans
{

	FrameBuffer::FrameBuffer(uint32_t width, uint32_t height)
		:m_Width(width), m_Height(height)
	{
		uint32_t renderbufer = 0;
		glGenFramebuffers(1, &m_RendererID);
		glGenRenderbuffers(1, &renderbufer);

		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		glBindRenderbuffer(GL_RENDERBUFFER, renderbufer);
		
		//ÏòframebufferÌí¼Óattachment
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, m_Width, m_Height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbufer);
		m_Renderbuffers.push_back(renderbufer);
	}

	FrameBuffer::~FrameBuffer()
	{
		glDeleteFramebuffers(1, &m_RendererID);
		glDeleteTextures(1, &m_ColorAttachment);
		glDeleteRenderbuffers(m_Renderbuffers.size(), m_Renderbuffers.data());
		m_Renderbuffers.clear();
		m_ColorAttachment = 0;
	}

	void FrameBuffer::Bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
	}

	void FrameBuffer::UnBind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void FrameBuffer::ActiveRenderBuffer(uint32_t index /*= 0*/)
	{
		if (index < m_Renderbuffers.size())
		{
			glBindRenderbuffer(GL_RENDERBUFFER, m_Renderbuffers[index]);
		}
	}
	void FrameBuffer::ResizeRenderBuffer(uint32_t width, uint32_t height, uint32_t index /*= 0*/)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		glBindRenderbuffer(GL_RENDERBUFFER, m_Renderbuffers[index]);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);

	}

	void FrameBuffer::SetColorAttachment(uint32_t renderid)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		m_ColorAttachment = renderid;
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderid, 0);
	}

}