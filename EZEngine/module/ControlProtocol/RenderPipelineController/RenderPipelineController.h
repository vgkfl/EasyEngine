#pragma once
#ifndef __RENDER_PIPELINE_CONTROLLER_H__
#define __RENDER_PIPELINE_CONTROLLER_H__

#include <vector>

#include "core/Render/IRenderFeature.h"
#include "ControlProtocol/RenderDeviceController/RenderDeviceController.h"
#include "ControlProtocol/RenderPipelineController/RenderImageBuffer.h"
#include "ControlProtocol/RenderPipelineController/RenderPassQueue.h"

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
			m_PassQueue.Clear();
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
			m_PassQueue.Clear();
			return device.BeginFrame();
		}

		void ExecutePipeline(
			EZ::ProjectContext& project,
			EZ::WorldContext& world,
			RenderImageBuffer& imageBuffer,
			RenderDeviceController& device)
		{
			RenderPassContext passContext{ project, world, imageBuffer, device };

			m_PassQueue.Clear();

			for (EZ::IRenderFeature* feature : m_Features)
			{
				if (!feature)
				{
					continue;
				}

				if (!feature->IsEnabled(passContext))
				{
					continue;
				}

				feature->CollectPasses(m_PassQueue, passContext);
			}

			m_PassQueue.Sort();

			for (EZ::IRenderPass* pass : m_PassQueue.GetPasses())
			{
				if (!pass)
				{
					continue;
				}

				pass->Setup(passContext);
				pass->Execute(passContext);
			}
		}

		// ¿œ–≠“È
		void ExecuteFeatures(
			EZ::ProjectContext& project,
			EZ::WorldContext& world,
			RenderImageBuffer& imageBuffer,
			RenderDeviceController& device)
		{
			ExecutePipeline(project, world, imageBuffer, device);
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

			m_PassQueue.Clear();
			device.EndFrame();
		}

	private:
		std::vector<EZ::IRenderFeature*> m_Features;
		RenderPassQueue m_PassQueue;
	};
}

#endif