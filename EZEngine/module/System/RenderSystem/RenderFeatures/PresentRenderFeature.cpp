#include "System/RenderSystem/RenderFeatures/PresentRenderFeature.h"

#include "ControlProtocol/RenderPipelineController/RenderPassQueue.h"

bool PresentRenderFeature::IsEnabled(const ControlProtocol::RenderPassContext& ctx) const
{
	(void)ctx;
	return m_Settings.enabled;
}

void PresentRenderFeature::CollectPasses(
	ControlProtocol::RenderPassQueue& queue,
	ControlProtocol::RenderPassContext& ctx)
{
	(void)ctx;
	queue.AddPass(m_PresentPass);
}