#pragma once

namespace Kans
{
	class FrameBuffer
	{
	public:
		FrameBuffer(uint32_t width, uint32_t height);
		~FrameBuffer();
		void Bind();
		void UnBind();
		void ActiveRenderBuffer(uint32_t index = 0);
		void ResizeRenderBuffer(uint32_t width, uint32_t height, uint32_t index = 0);
		void SetColorAttachment(uint32_t renderid);

	private:
		uint32_t m_RendererID;
		uint32_t m_ColorAttachment;
		std::vector<uint32_t> m_Renderbuffers;
		uint32_t m_Height;
		uint32_t m_Width;

	};
}