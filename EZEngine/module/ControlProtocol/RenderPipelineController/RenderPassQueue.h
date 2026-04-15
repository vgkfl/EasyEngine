#pragma once
#ifndef __RENDER_PASS_QUEUE_H__
#define __RENDER_PASS_QUEUE_H__

#include <algorithm>
#include <vector>

#include "core/Render/IRenderPass.h"
#include "core/Context/RunTimeContext/ProjectContext.h"
#include "core/Context/RunTimeContext/WorldContext.h"
#include "ControlProtocol/RenderPipelineController/RenderImageBuffer.h"
#include "ControlProtocol/RenderDeviceController/RenderDeviceController.h"

namespace ControlProtocol
{
	struct RenderPassContext
	{
		EZ::ProjectContext& project;
		EZ::WorldContext& world;
		RenderImageBuffer& imageBuffer;
		RenderDeviceController& device;
	};

	class RenderPassQueue
	{
	public:
		void Clear()
		{
			m_Passes.clear();
		}

		void AddPass(EZ::IRenderPass& pass)
		{
			m_Passes.push_back(&pass);
		}

		void Sort()
		{
			std::stable_sort(
				m_Passes.begin(),
				m_Passes.end(),
				[](const EZ::IRenderPass* lhs, const EZ::IRenderPass* rhs)
				{
					if (lhs == rhs)
					{
						return false;
					}
					if (!lhs)
					{
						return false;
					}
					if (!rhs)
					{
						return true;
					}

					const EZ::u16 lhsEvent = static_cast<EZ::u16>(lhs->GetPassEvent());
					const EZ::u16 rhsEvent = static_cast<EZ::u16>(rhs->GetPassEvent());

					if (lhsEvent != rhsEvent)
					{
						return lhsEvent < rhsEvent;
					}

					return lhs->GetPassOrder() < rhs->GetPassOrder();
				});
		}

		const std::vector<EZ::IRenderPass*>& GetPasses() const
		{
			return m_Passes;
		}

	private:
		std::vector<EZ::IRenderPass*> m_Passes;
	};
}

#endif