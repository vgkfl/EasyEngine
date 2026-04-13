#pragma once
#ifndef __CORE_RENDER_IMAGE_DESC_H__
#define __CORE_RENDER_IMAGE_DESC_H__

#include "core/Render/RenderTypes.h"

namespace EZ
{
	struct RenderImageDesc
	{
		RenderImageTag tag = RenderImageTag::Unknown;
		RenderSourceType source = RenderSourceType::Unknown;

		RenderExtent2D extent{};
		RenderFormat format = RenderFormat::Unknown;

		bool valid = false;
		bool imported = false;
		bool persistent = false;

		bool sampled = true;
		bool colorAttachment = true;
		bool depthStencilAttachment = false;

		RenderLoadAction loadAction = RenderLoadAction::Clear;
		RenderStoreAction storeAction = RenderStoreAction::Store;
		RenderClearValue clearValue{};
	};
}

#endif