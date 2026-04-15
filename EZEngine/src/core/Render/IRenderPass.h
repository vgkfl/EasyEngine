#pragma once
#ifndef __CORE_I_RENDER_PASS_H__
#define __CORE_I_RENDER_PASS_H__

#include "core/Types.h"
#include "core/Render/RenderPassEvent.h"

namespace ControlProtocol
{
	struct RenderPassContext;
}

namespace EZ
{
	class IRenderPass
	{
	public:
		virtual ~IRenderPass() = default;

		virtual const char* GetName() const = 0;
		virtual RenderPassEvent GetPassEvent() const = 0;

		virtual EZ::u16 GetPassOrder() const
		{
			return 0;
		}

		virtual void Setup(ControlProtocol::RenderPassContext& ctx) = 0;
		virtual void Execute(ControlProtocol::RenderPassContext& ctx) = 0;
	};
}

#endif