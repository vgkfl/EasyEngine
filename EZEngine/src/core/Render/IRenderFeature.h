#pragma once
#ifndef __CORE_I_RENDER_FEATURE_H__
#define __CORE_I_RENDER_FEATURE_H__

namespace EZ
{
	struct ProjectContext;
	struct WorldContext;
}

namespace ControlProtocol
{
	class RenderImageBuffer;
	class RenderDeviceController;
}

namespace EZ
{
	class IRenderFeature
	{
	public:
		virtual ~IRenderFeature() = default;

		virtual const char* GetName() const = 0;
		virtual bool IsEnabled() const = 0;

		virtual void Setup(
			ProjectContext& project,
			WorldContext& world,
			ControlProtocol::RenderImageBuffer& imageBuffer,
			ControlProtocol::RenderDeviceController& device) = 0;

		virtual void Execute(
			ProjectContext& project,
			WorldContext& world,
			ControlProtocol::RenderImageBuffer& imageBuffer,
			ControlProtocol::RenderDeviceController& device) = 0;
	};
}

#endif