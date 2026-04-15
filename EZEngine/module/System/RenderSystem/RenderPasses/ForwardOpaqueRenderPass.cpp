#include "System/RenderSystem/RenderPasses/ForwardOpaqueRenderPass.h"

#include "System/RenderSystem/RenderSystem.h"
#include "ControlProtocol/RenderPipelineController/RenderPassQueue.h"

void ForwardOpaqueRenderPass::Setup(ControlProtocol::RenderPassContext& ctx)
{
	auto* renderSystem = ctx.world.TryGet<RenderSystem>();
	if (!renderSystem)
	{
		return;
	}

	renderSystem->SetupForwardOpaqueTargets(
		ctx.world,
		ctx.imageBuffer,
		ctx.device);
}

void ForwardOpaqueRenderPass::Execute(ControlProtocol::RenderPassContext& ctx)
{
	auto* renderSystem = ctx.world.TryGet<RenderSystem>();
	if (!renderSystem)
	{
		return;
	}

	renderSystem->ExecuteForwardOpaque(
		ctx.world,
		ctx.imageBuffer,
		ctx.device);
}