#include "System/RenderSystem/RenderFeatures/PresentRenderFeature.h"

#include "System/RenderSystem/RenderSystem.h"
#include "core/Context/RunTimeContext/ProjectContext.h"
#include "core/Context/RunTimeContext/WorldContext.h"

void PresentRenderFeature::Setup(
	EZ::ProjectContext& project,
	EZ::WorldContext& world,
	ControlProtocol::RenderImageBuffer& imageBuffer,
	ControlProtocol::RenderDeviceController& device)
{
	(void)project;
	(void)world;
	(void)imageBuffer;
	(void)device;
}

void PresentRenderFeature::Execute(
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

	renderSystem->Present(world, imageBuffer, device);
}