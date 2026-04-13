#pragma once
#ifndef __GL_RENDER_DEVICE_CONTROLLER_H__
#define __GL_RENDER_DEVICE_CONTROLLER_H__

#include <SDL3/SDL.h>

#include "GL/glew.h"

#include "ControlProtocol/RenderDeviceController/RenderDeviceController.h"

namespace ControlProtocol
{
	class GLRenderDeviceController final : public RenderDeviceController
	{
	public:
		GLRenderDeviceController() = default;
		~GLRenderDeviceController() override;

		bool Initialize(WindowController& window) override;
		void Shutdown() override;

		bool CreateImage(RenderImageSlot& slot) override;
		void DestroyImage(RenderImageSlot& slot) override;

		bool CreateRenderTarget(RenderImageSlot& slot) override;
		void DestroyRenderTarget(RenderImageSlot& slot) override;

		bool EnsureImage(RenderImageSlot& slot) override;

		bool BeginFrame() override;
		void EndFrame() override;

		bool BeginPass(RenderImageSlot* colorSlot, RenderImageSlot* depthSlot) override;
		void EndPass() override;

		void ClearColor(RenderImageSlot& slot, const EZ::RenderClearColorValue& clearValue) override;
		void ClearDepth(RenderImageSlot& slot, const EZ::RenderClearDepthStencilValue& clearValue) override;

		bool Blit(const RenderImageSlot& src, RenderImageSlot& dst) override;
		bool Present(WindowController& window, const RenderImageSlot& finalColor) override;

	private:
		bool EnsureContext(WindowController& window);
		bool CheckFramebufferComplete() const;

		static bool IsDepthFormat(EZ::RenderFormat format);
		static bool IsDepthStencilFormat(EZ::RenderFormat format);
		static GLenum ToGLInternalFormat(EZ::RenderFormat format);
		static GLenum ToGLFormat(EZ::RenderFormat format);
		static GLenum ToGLType(EZ::RenderFormat format);

		static GLuint ToGLHandle(EZ::u64 handle)
		{
			return static_cast<GLuint>(handle);
		}

		static EZ::u64 ToRenderHandle(GLuint handle)
		{
			return static_cast<EZ::u64>(handle);
		}

	private:
		SDL_Window* m_Window = nullptr;
		SDL_GLContext m_GLContext = nullptr;
		bool m_GlewInitialized = false;
		bool m_FrameActive = false;
	};
}

#endif