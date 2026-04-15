#include "Tool/Editor/RenderPasses/CopyToFinalRenderPass.h"

#include "ControlProtocol/RenderPipelineController/RenderPassQueue.h"
#include "core/Render/RenderTypes.h"

namespace
{
	bool EnsurePersistentFrameTarget(
		EZ::RenderImageTag tag,
		const EZ::RenderImageDesc& desc,
		ControlProtocol::RenderImageBuffer& imageBuffer,
		ControlProtocol::RenderDeviceController& device)
	{
		auto& slot = imageBuffer.GetOrCreate(tag);

		const bool needRecreate =
			slot.desc.extent.width != desc.extent.width ||
			slot.desc.extent.height != desc.extent.height ||
			slot.desc.format != desc.format ||
			slot.desc.imported != desc.imported;

		if (needRecreate)
		{
			device.DestroyRenderTarget(slot);
			device.DestroyImage(slot);
		}

		slot.desc = desc;
		slot.desc.tag = tag;
		slot.desc.valid = true;
		slot.desc.persistent = true;

		return device.EnsureImage(slot);
	}
}

namespace Tool
{
	void CopyToFinalRenderPass::Setup(ControlProtocol::RenderPassContext& ctx)
	{
		auto* src = ctx.imageBuffer.TryGet(EZ::RenderImageTag::ForwardOpaque);
		if (!src)
		{
			return;
		}

		EZ::RenderImageDesc finalDesc = src->desc;
		finalDesc.source = EZ::RenderSourceType::UIPass;
		finalDesc.tag = EZ::RenderImageTag::FinalColor;
		finalDesc.valid = true;
		finalDesc.imported = false;
		finalDesc.persistent = true;
		finalDesc.sampled = true;
		finalDesc.colorAttachment = true;
		finalDesc.depthStencilAttachment = false;
		finalDesc.loadAction = EZ::RenderLoadAction::Load;
		finalDesc.storeAction = EZ::RenderStoreAction::Store;

		EnsurePersistentFrameTarget(
			EZ::RenderImageTag::FinalColor,
			finalDesc,
			ctx.imageBuffer,
			ctx.device);
	}

	void CopyToFinalRenderPass::Execute(ControlProtocol::RenderPassContext& ctx)
	{
		auto* src = ctx.imageBuffer.TryGet(EZ::RenderImageTag::ForwardOpaque);
		auto* dst = ctx.imageBuffer.TryGet(EZ::RenderImageTag::FinalColor);
		if (!src || !dst)
		{
			return;
		}

		ctx.device.Blit(*src, *dst);
	}
}