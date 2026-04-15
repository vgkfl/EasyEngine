#pragma once
#ifndef __CORE_I_RENDER_FEATURE_H__
#define __CORE_I_RENDER_FEATURE_H__

namespace ControlProtocol
{
	class RenderPassQueue;
	struct RenderPassContext;
}

namespace EZ
{
	class IRenderFeature
	{
	public:
		virtual ~IRenderFeature() = default;

		virtual const char* GetName() const = 0;
		virtual bool IsEnabled(const ControlProtocol::RenderPassContext& ctx) const = 0;

		virtual void CollectPasses(
			ControlProtocol::RenderPassQueue& queue,
			ControlProtocol::RenderPassContext& ctx) = 0;
	};
}

#endif