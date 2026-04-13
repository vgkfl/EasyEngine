#include "System/RenderSystem/RenderFeatures/ForwardOpaqueRenderFeature.h"

#include "System/RenderSystem/RenderSystem.h"
#include "core/Context/RunTimeContext/ProjectContext.h"
#include "core/Context/RunTimeContext/WorldContext.h"

void ForwardOpaqueRenderFeature::Setup(
	EZ::ProjectContext& project,
	EZ::WorldContext& world,
	ControlProtocol::RenderImageBuffer& imageBuffer,
	ControlProtocol::RenderDeviceController& device)
{
	(void)project;

	auto* renderSystem = world.TryGet<RenderSystem>();
	if (!renderSystem)
	{
		return;
	}

	renderSystem->SetupForwardOpaqueTargets(world, imageBuffer, device);
}

void ForwardOpaqueRenderFeature::Execute(
	EZ::ProjectContext& project,
	EZ::WorldContext& world,
	ControlProtocol::RenderImageBuffer& imageBuffer,
	ControlProtocol::RenderDeviceController& device)
{
	(void)project;

	auto* renderSystem = world.TryGet<RenderSystem>();
	if (!renderSystem)
	{
		return;
	}

	renderSystem->ExecuteForwardOpaque(world, imageBuffer, device);
}