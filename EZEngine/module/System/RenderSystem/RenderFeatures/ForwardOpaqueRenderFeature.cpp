#include "System/RenderSystem/RenderFeatures/ForwardOpaqueRenderFeature.h"

#include "ControlProtocol/RenderPipelineController/RenderPassQueue.h"

bool ForwardOpaqueRenderFeature::IsEnabled(const ControlProtocol::RenderPassContext& ctx) const
{
	(void)ctx;
	return m_Settings.enabled;
}

void ForwardOpaqueRenderFeature::CollectPasses(
	ControlProtocol::RenderPassQueue& queue,
	ControlProtocol::RenderPassContext& ctx)
{
	(void)ctx;
	queue.AddPass(m_ForwardOpaquePass);
}