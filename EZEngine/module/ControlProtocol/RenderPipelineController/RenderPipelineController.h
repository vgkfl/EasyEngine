#pragma once
#ifndef __RENDER_PIPELINE_CONTROLLER_H__
#define __RENDER_PIPELINE_CONTROLLER_H__

#include <vector>

#include "core/Render/IRenderFeature.h"
#include "ControlProtocol/RenderDeviceController/RenderDeviceController.h"
#include "ControlProtocol/RenderPipelineController/RenderImageBuffer.h"

namespace ControlProtocol
{
	class RenderPipelineController
	{
	public:
		void AddFeature(EZ::IRenderFeature& feature)
		{
			m_Features.push_back(&feature);
		}

		void ClearFeatures()
		{
			m_Features.clear();
		}

		bool BeginFrame(
			EZ::ProjectContext& project,
			EZ::WorldContext& world,
			RenderImageBuffer& imageBuffer,
			RenderDeviceController& device)
		{
			(void)project;
			(void)world;
			imageBuffer.ResetFrame();
			return device.BeginFrame();
		}

		void ExecuteFeatures(
			EZ::ProjectContext& project,
			EZ::WorldContext& world,
			RenderImageBuffer& imageBuffer,
			RenderDeviceController& device)
		{
			for (EZ::IRenderFeature* feature : m_Features)
			{
				if (!feature || !feature->IsEnabled())
				{
					continue;
				}

				feature->Setup(project, world, imageBuffer, device);
				feature->Execute(project, world, imageBuffer, device);
			}
		}

		void EndFrame(
			EZ::ProjectContext& project,
			EZ::WorldContext& world,
			RenderImageBuffer& imageBuffer,
			RenderDeviceController& device)
		{
			(void)project;
			(void)world;
			(void)imageBuffer;
			device.EndFrame();
		}

	private:
		std::vector<EZ::IRenderFeature*> m_Features;
	};
}

#endif