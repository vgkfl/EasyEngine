#pragma once
#ifndef __CORE_RENDER_PASS_EVENT_H__
#define __CORE_RENDER_PASS_EVENT_H__

#include "core/Types.h"

namespace EZ
{
	enum class RenderPassEvent : EZ::u16
	{
		BeforeShadow = 100,
		Shadow = 200,

		BeforePrepass = 300,
		Prepass = 400,

		BeforeOpaque = 500,
		Opaque = 600,
		AfterOpaque = 700,

		BeforeTransparent = 800,
		Transparent = 900,
		AfterTransparent = 1000,

		BeforePostProcess = 1100,
		PostProcess = 1200,

		Overlay = 1300,
		Present = 1400
	};
}

#endif