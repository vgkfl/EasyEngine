#pragma once
#ifndef __CORE_RENDER_TYPES_H__
#define __CORE_RENDER_TYPES_H__

#include "core/Types.h"

namespace EZ
{
	struct RenderExtent2D
	{
		EZ::u32 width = 0;
		EZ::u32 height = 0;
	};

	enum class RenderFormat : EZ::u16
	{
		Unknown = 0,

		R8_UNorm,
		RG8_UNorm,
		RGBA8_UNorm,
		RGBA8_SRGB,

		R16_Float,
		RG16_Float,
		RGBA16_Float,

		R32_Float,
		RG32_Float,
		RGBA32_Float,

		D16_UNorm,
		D24_UNorm_S8_UInt,
		D32_Float
	};

	enum class RenderImageTag : EZ::u16
	{
		Unknown = 0,

		SceneColor,
		SceneDepth,
		DepthNormal,

		ShadowDepth_MainLight,
		ShadowDepth_PerObject,

		ForwardOpaque,
		ForwardTransparent,

		DebugPreview,
		FinalColor,

		Count
	};

	enum class RenderSourceType : EZ::u8
	{
		Unknown = 0,
		Imported,
		ForwardPass,
		DeferredPass,
		ShadowPass,
		PostProcessPass,
		DebugPass,
		UIPass
	};

	enum class RenderLoadAction : EZ::u8
	{
		DontCare = 0,
		Load,
		Clear
	};

	enum class RenderStoreAction : EZ::u8
	{
		DontCare = 0,
		Store
	};

	struct RenderClearColorValue
	{
		float r = 0.0f;
		float g = 0.0f;
		float b = 0.0f;
		float a = 1.0f;
	};

	struct RenderClearDepthStencilValue
	{
		float depth = 1.0f;
		EZ::u32 stencil = 0;
	};

	struct RenderClearValue
	{
		RenderClearColorValue color;
		RenderClearDepthStencilValue depthStencil;
	};
}

#endif