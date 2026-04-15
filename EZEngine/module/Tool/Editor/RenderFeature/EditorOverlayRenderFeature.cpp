#include "Tool/Editor/RenderFeature/EditorOverlayRenderFeature.h"

#include "ControlProtocol/RenderPipelineController/RenderPassQueue.h"
#include "Tool/Editor/EditorContext.h"

namespace Tool
{
	bool EditorOverlayRenderFeature::IsEnabled(const ControlProtocol::RenderPassContext& ctx) const
	{
		auto* editor = ctx.world.TryGet<Tool::EditorContext>();
		return editor && editor->enabled;
	}

	void EditorOverlayRenderFeature::CollectPasses(
		ControlProtocol::RenderPassQueue& queue,
		ControlProtocol::RenderPassContext& ctx)
	{
		(void)ctx;

		queue.AddPass(m_CopyToFinalPass);
		queue.AddPass(m_EditorOverlayPass);
	}
}