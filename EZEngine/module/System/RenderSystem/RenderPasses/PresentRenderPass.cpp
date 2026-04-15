#include "System/RenderSystem/RenderPasses/PresentRenderPass.h"

#include "System/RenderSystem/RenderSystem.h"
#include "ControlProtocol/RenderPipelineController/RenderPassQueue.h"

void PresentRenderPass::Setup(ControlProtocol::RenderPassContext& ctx)
{
	(void)ctx;
}

void PresentRenderPass::Execute(ControlProtocol::RenderPassContext& ctx)
{
	auto* renderSystem = ctx.world.TryGet<RenderSystem>();
	if (!renderSystem)
	{
		return;
	}

	renderSystem->Present(
		ctx.world,
		ctx.imageBuffer,
		ctx.device);
}