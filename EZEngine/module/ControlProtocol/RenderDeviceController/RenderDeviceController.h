#pragma once
#ifndef __RENDER_DEVICE_CONTROLLER_H__
#define __RENDER_DEVICE_CONTROLLER_H__

#include "ControlProtocol/RenderPipelineController/RenderImageBuffer.h"
#include "core/Render/RenderTypes.h"

namespace ControlProtocol
{
	class WindowController;

	class RenderDeviceController
	{
	public:
		virtual ~RenderDeviceController() = default;

		virtual bool Initialize(WindowController& window) = 0;
		virtual void Shutdown() = 0;

		virtual bool CreateImage(RenderImageSlot& slot) = 0;
		virtual void DestroyImage(RenderImageSlot& slot) = 0;

		virtual bool CreateRenderTarget(RenderImageSlot& slot) = 0;
		virtual void DestroyRenderTarget(RenderImageSlot& slot) = 0;

		virtual bool EnsureImage(RenderImageSlot& slot) = 0;

		virtual bool BeginFrame() = 0;
		virtual void EndFrame() = 0;

		virtual bool BeginPass(RenderImageSlot* colorSlot, RenderImageSlot* depthSlot) = 0;
		virtual void EndPass() = 0;

		virtual void ClearColor(RenderImageSlot& slot, const EZ::RenderClearColorValue& clearValue) = 0;
		virtual void ClearDepth(RenderImageSlot& slot, const EZ::RenderClearDepthStencilValue& clearValue) = 0;

		virtual bool Blit(const RenderImageSlot& src, RenderImageSlot& dst) = 0;
		virtual bool Present(WindowController& window, const RenderImageSlot& finalColor) = 0;
	};
}

#endif