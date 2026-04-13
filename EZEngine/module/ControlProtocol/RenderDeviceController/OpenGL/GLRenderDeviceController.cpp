#include "GLRenderDeviceController.h"

#include <GL/glew.h>

#include <algorithm>
#include <cstdio>

#include "ControlProtocol/WindowController/WindowController.h"

namespace
{
	void BindFramebuffer(GLuint fbo)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	}

	void BindReadFramebuffer(GLuint fbo)
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
	}

	void BindDrawFramebuffer(GLuint fbo)
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	}
}

namespace ControlProtocol
{
	GLRenderDeviceController::~GLRenderDeviceController()
	{
		Shutdown();
	}

	bool GLRenderDeviceController::Initialize(WindowController& window)
	{
		return EnsureContext(window);
	}

	void GLRenderDeviceController::Shutdown()
	{
		if (m_GLContext && m_Window)
		{
			SDL_GL_MakeCurrent(m_Window, m_GLContext);
		}

		m_FrameActive = false;
		m_GlewInitialized = false;

		if (m_GLContext)
		{
			SDL_GL_DestroyContext(m_GLContext);
			m_GLContext = nullptr;
		}

		m_Window = nullptr;
	}

	bool GLRenderDeviceController::EnsureContext(WindowController& window)
	{
		if (m_GLContext && m_Window == window.GetBackendWindowHandle())
		{
			if (!SDL_GL_MakeCurrent(m_Window, m_GLContext))
			{
				return false;
			}
			return true;
		}

		SDL_Window* sdlWindow = static_cast<SDL_Window*>(window.GetBackendWindowHandle());
		if (!sdlWindow)
		{
			return false;
		}

		if (m_GLContext)
		{
			SDL_GL_DestroyContext(m_GLContext);
			m_GLContext = nullptr;
			m_GlewInitialized = false;
		}

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

		m_GLContext = SDL_GL_CreateContext(sdlWindow);
		if (!m_GLContext)
		{
			return false;
		}

		if (!SDL_GL_MakeCurrent(sdlWindow, m_GLContext))
		{
			SDL_GL_DestroyContext(m_GLContext);
			m_GLContext = nullptr;
			return false;
		}

		SDL_GL_SetSwapInterval(0);

		if (!m_GlewInitialized)
		{
			glewExperimental = GL_TRUE;
			const GLenum glewResult = glewInit();
			glGetError();

			if (glewResult != GLEW_OK)
			{
				SDL_GL_DestroyContext(m_GLContext);
				m_GLContext = nullptr;
				return false;
			}

			m_GlewInitialized = true;
		}

		m_Window = sdlWindow;

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);

		return true;
	}

	bool GLRenderDeviceController::IsDepthFormat(EZ::RenderFormat format)
	{
		return format == EZ::RenderFormat::D16_UNorm ||
			format == EZ::RenderFormat::D24_UNorm_S8_UInt ||
			format == EZ::RenderFormat::D32_Float;
	}

	bool GLRenderDeviceController::IsDepthStencilFormat(EZ::RenderFormat format)
	{
		return format == EZ::RenderFormat::D24_UNorm_S8_UInt;
	}

	GLenum GLRenderDeviceController::ToGLInternalFormat(EZ::RenderFormat format)
	{
		switch (format)
		{
		case EZ::RenderFormat::R8_UNorm:           return GL_R8;
		case EZ::RenderFormat::RG8_UNorm:          return GL_RG8;
		case EZ::RenderFormat::RGBA8_UNorm:        return GL_RGBA8;
		case EZ::RenderFormat::RGBA8_SRGB:         return GL_SRGB8_ALPHA8;

		case EZ::RenderFormat::R16_Float:          return GL_R16F;
		case EZ::RenderFormat::RG16_Float:         return GL_RG16F;
		case EZ::RenderFormat::RGBA16_Float:       return GL_RGBA16F;

		case EZ::RenderFormat::R32_Float:          return GL_R32F;
		case EZ::RenderFormat::RG32_Float:         return GL_RG32F;
		case EZ::RenderFormat::RGBA32_Float:       return GL_RGBA32F;

		case EZ::RenderFormat::D16_UNorm:          return GL_DEPTH_COMPONENT16;
		case EZ::RenderFormat::D24_UNorm_S8_UInt:  return GL_DEPTH24_STENCIL8;
		case EZ::RenderFormat::D32_Float:          return GL_DEPTH_COMPONENT32F;

		default:                                   return 0;
		}
	}

	GLenum GLRenderDeviceController::ToGLFormat(EZ::RenderFormat format)
	{
		switch (format)
		{
		case EZ::RenderFormat::R8_UNorm:
		case EZ::RenderFormat::R16_Float:
		case EZ::RenderFormat::R32_Float:
			return GL_RED;

		case EZ::RenderFormat::RG8_UNorm:
		case EZ::RenderFormat::RG16_Float:
		case EZ::RenderFormat::RG32_Float:
			return GL_RG;

		case EZ::RenderFormat::RGBA8_UNorm:
		case EZ::RenderFormat::RGBA8_SRGB:
		case EZ::RenderFormat::RGBA16_Float:
		case EZ::RenderFormat::RGBA32_Float:
			return GL_RGBA;

		case EZ::RenderFormat::D16_UNorm:
		case EZ::RenderFormat::D32_Float:
			return GL_DEPTH_COMPONENT;

		case EZ::RenderFormat::D24_UNorm_S8_UInt:
			return GL_DEPTH_STENCIL;

		default:
			return 0;
		}
	}

	GLenum GLRenderDeviceController::ToGLType(EZ::RenderFormat format)
	{
		switch (format)
		{
		case EZ::RenderFormat::R8_UNorm:
		case EZ::RenderFormat::RG8_UNorm:
		case EZ::RenderFormat::RGBA8_UNorm:
		case EZ::RenderFormat::RGBA8_SRGB:
			return GL_UNSIGNED_BYTE;

		case EZ::RenderFormat::R16_Float:
		case EZ::RenderFormat::RG16_Float:
		case EZ::RenderFormat::RGBA16_Float:
		case EZ::RenderFormat::R32_Float:
		case EZ::RenderFormat::RG32_Float:
		case EZ::RenderFormat::RGBA32_Float:
		case EZ::RenderFormat::D32_Float:
			return GL_FLOAT;

		case EZ::RenderFormat::D16_UNorm:
			return GL_UNSIGNED_SHORT;

		case EZ::RenderFormat::D24_UNorm_S8_UInt:
			return GL_UNSIGNED_INT_24_8;

		default:
			return 0;
		}
	}

	bool GLRenderDeviceController::CreateImage(RenderImageSlot& slot)
	{
		if (!m_GLContext || !slot.desc.valid || slot.desc.imported)
		{
			return slot.desc.imported;
		}

		if (slot.imageHandle != 0)
		{
			return true;
		}

		const GLenum internalFormat = ToGLInternalFormat(slot.desc.format);
		const GLenum format = ToGLFormat(slot.desc.format);
		const GLenum type = ToGLType(slot.desc.format);

		if (internalFormat == 0 || format == 0 || type == 0)
		{
			return false;
		}

		if (slot.desc.extent.width == 0 || slot.desc.extent.height == 0)
		{
			return false;
		}

		GLuint texture = 0;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		if (IsDepthFormat(slot.desc.format))
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
		}

		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			static_cast<GLint>(internalFormat),
			static_cast<GLsizei>(slot.desc.extent.width),
			static_cast<GLsizei>(slot.desc.extent.height),
			0,
			format,
			type,
			nullptr);

		glBindTexture(GL_TEXTURE_2D, 0);

		slot.imageHandle = ToRenderHandle(texture);
		return true;
	}

	void GLRenderDeviceController::DestroyImage(RenderImageSlot& slot)
	{
		if (!m_GLContext || slot.desc.imported)
		{
			slot.imageHandle = 0;
			return;
		}

		if (slot.imageHandle != 0)
		{
			const GLuint texture = ToGLHandle(slot.imageHandle);
			glDeleteTextures(1, &texture);
			slot.imageHandle = 0;
		}
	}

	bool GLRenderDeviceController::CreateRenderTarget(RenderImageSlot& slot)
	{
		if (!m_GLContext || !slot.desc.valid || slot.desc.imported)
		{
			return slot.desc.imported;
		}

		if (slot.renderTargetHandle != 0)
		{
			return true;
		}

		if (slot.imageHandle == 0 && !CreateImage(slot))
		{
			return false;
		}

		GLuint fbo = 0;
		glGenFramebuffers(1, &fbo);
		BindFramebuffer(fbo);

		const GLuint texture = ToGLHandle(slot.imageHandle);

		if (IsDepthFormat(slot.desc.format))
		{
			if (IsDepthStencilFormat(slot.desc.format))
			{
				glFramebufferTexture2D(
					GL_FRAMEBUFFER,
					GL_DEPTH_STENCIL_ATTACHMENT,
					GL_TEXTURE_2D,
					texture,
					0);
			}
			else
			{
				glFramebufferTexture2D(
					GL_FRAMEBUFFER,
					GL_DEPTH_ATTACHMENT,
					GL_TEXTURE_2D,
					texture,
					0);
			}

			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
		}
		else
		{
			glFramebufferTexture2D(
				GL_FRAMEBUFFER,
				GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_2D,
				texture,
				0);

			const GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
			glDrawBuffers(1, drawBuffers);
		}

		if (!CheckFramebufferComplete())
		{
			BindFramebuffer(0);
			glDeleteFramebuffers(1, &fbo);
			return false;
		}

		BindFramebuffer(0);
		slot.renderTargetHandle = ToRenderHandle(fbo);
		return true;
	}

	void GLRenderDeviceController::DestroyRenderTarget(RenderImageSlot& slot)
	{
		if (!m_GLContext || slot.desc.imported)
		{
			slot.renderTargetHandle = 0;
			return;
		}

		if (slot.renderTargetHandle != 0)
		{
			const GLuint fbo = ToGLHandle(slot.renderTargetHandle);
			glDeleteFramebuffers(1, &fbo);
			slot.renderTargetHandle = 0;
		}
	}

	bool GLRenderDeviceController::EnsureImage(RenderImageSlot& slot)
	{
		if (!slot.desc.valid)
		{
			return false;
		}

		if (!slot.desc.imported)
		{
			if (slot.imageHandle == 0 && !CreateImage(slot))
			{
				return false;
			}

			if (slot.renderTargetHandle == 0 && !CreateRenderTarget(slot))
			{
				return false;
			}
		}

		return true;
	}

	bool GLRenderDeviceController::BeginFrame()
	{
		if (!m_GLContext)
		{
			return false;
		}

		m_FrameActive = true;
		return true;
	}

	void GLRenderDeviceController::EndFrame()
	{
		m_FrameActive = false;
		BindFramebuffer(0);
	}

	bool GLRenderDeviceController::CheckFramebufferComplete() const
	{
		return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
	}

	bool GLRenderDeviceController::BeginPass(RenderImageSlot* colorSlot, RenderImageSlot* depthSlot)
	{
		if (!m_GLContext || !m_FrameActive)
		{
			return false;
		}

		if (colorSlot && !EnsureImage(*colorSlot))
		{
			return false;
		}

		if (depthSlot && !EnsureImage(*depthSlot))
		{
			return false;
		}

		if (!colorSlot && !depthSlot)
		{
			BindFramebuffer(0);
			return true;
		}

		GLuint targetFbo = 0;

		if (colorSlot && colorSlot->renderTargetHandle != 0)
		{
			targetFbo = ToGLHandle(colorSlot->renderTargetHandle);
			BindFramebuffer(targetFbo);

			if (depthSlot && depthSlot->imageHandle != 0 && IsDepthFormat(depthSlot->desc.format))
			{
				const GLuint depthTexture = ToGLHandle(depthSlot->imageHandle);

				if (IsDepthStencilFormat(depthSlot->desc.format))
				{
					glFramebufferTexture2D(
						GL_FRAMEBUFFER,
						GL_DEPTH_STENCIL_ATTACHMENT,
						GL_TEXTURE_2D,
						depthTexture,
						0);
				}
				else
				{
					glFramebufferTexture2D(
						GL_FRAMEBUFFER,
						GL_DEPTH_ATTACHMENT,
						GL_TEXTURE_2D,
						depthTexture,
						0);
				}
			}
		}
		else if (depthSlot && depthSlot->renderTargetHandle != 0)
		{
			targetFbo = ToGLHandle(depthSlot->renderTargetHandle);
			BindFramebuffer(targetFbo);

			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
		}
		else
		{
			return false;
		}

		if (!CheckFramebufferComplete())
		{
			BindFramebuffer(0);
			return false;
		}

		EZ::u32 width = 0;
		EZ::u32 height = 0;

		if (colorSlot)
		{
			width = colorSlot->desc.extent.width;
			height = colorSlot->desc.extent.height;
		}
		else if (depthSlot)
		{
			width = depthSlot->desc.extent.width;
			height = depthSlot->desc.extent.height;
		}

		glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
		return true;
	}

	void GLRenderDeviceController::EndPass()
	{
		BindFramebuffer(0);
	}

	void GLRenderDeviceController::ClearColor(RenderImageSlot& slot, const EZ::RenderClearColorValue& clearValue)
	{
		if (!m_GLContext || slot.renderTargetHandle == 0)
		{
			return;
		}

		const GLuint fbo = ToGLHandle(slot.renderTargetHandle);
		BindFramebuffer(fbo);

		glViewport(
			0,
			0,
			static_cast<GLsizei>(slot.desc.extent.width),
			static_cast<GLsizei>(slot.desc.extent.height));

		glClearColor(clearValue.r, clearValue.g, clearValue.b, clearValue.a);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void GLRenderDeviceController::ClearDepth(RenderImageSlot& slot, const EZ::RenderClearDepthStencilValue& clearValue)
	{
		if (!m_GLContext || slot.renderTargetHandle == 0)
		{
			return;
		}

		const GLuint fbo = ToGLHandle(slot.renderTargetHandle);
		BindFramebuffer(fbo);

		glViewport(
			0,
			0,
			static_cast<GLsizei>(slot.desc.extent.width),
			static_cast<GLsizei>(slot.desc.extent.height));

		glClearDepth(clearValue.depth);

		if (IsDepthStencilFormat(slot.desc.format))
		{
			glClearStencil(static_cast<GLint>(clearValue.stencil));
			glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		}
		else
		{
			glClear(GL_DEPTH_BUFFER_BIT);
		}
	}

	bool GLRenderDeviceController::Blit(const RenderImageSlot& src, RenderImageSlot& dst)
	{
		if (!m_GLContext)
		{
			return false;
		}

		if (src.renderTargetHandle == 0)
		{
			return false;
		}

		if (!EnsureImage(dst))
		{
			return false;
		}

		const GLuint srcFbo = ToGLHandle(src.renderTargetHandle);
		const GLuint dstFbo = ToGLHandle(dst.renderTargetHandle);

		BindReadFramebuffer(srcFbo);
		BindDrawFramebuffer(dstFbo);

		const GLbitfield mask = IsDepthFormat(src.desc.format) ? GL_DEPTH_BUFFER_BIT : GL_COLOR_BUFFER_BIT;
		glBlitFramebuffer(
			0, 0,
			static_cast<GLint>(src.desc.extent.width),
			static_cast<GLint>(src.desc.extent.height),
			0, 0,
			static_cast<GLint>(dst.desc.extent.width),
			static_cast<GLint>(dst.desc.extent.height),
			mask,
			GL_NEAREST);

		BindReadFramebuffer(0);
		BindDrawFramebuffer(0);
		return true;
	}

	bool GLRenderDeviceController::Present(WindowController& window, const RenderImageSlot& finalColor)
	{
		if (!m_GLContext)
		{
			return false;
		}

		if (!EnsureContext(window))
		{
			return false;
		}

		if (finalColor.renderTargetHandle == 0)
		{
			SDL_GL_SwapWindow(static_cast<SDL_Window*>(window.GetBackendWindowHandle()));
			return true;
		}

		const DataProtocol::UVec2 drawableSize = window.GetDrawableSize();

		BindReadFramebuffer(ToGLHandle(finalColor.renderTargetHandle));
		BindDrawFramebuffer(0);

		glBlitFramebuffer(
			0, 0,
			static_cast<GLint>(finalColor.desc.extent.width),
			static_cast<GLint>(finalColor.desc.extent.height),
			0, 0,
			static_cast<GLint>(drawableSize.x),
			static_cast<GLint>(drawableSize.y),
			GL_COLOR_BUFFER_BIT,
			GL_NEAREST);

		BindReadFramebuffer(0);
		BindDrawFramebuffer(0);

		SDL_GL_SwapWindow(static_cast<SDL_Window*>(window.GetBackendWindowHandle()));
		return true;
	}
}