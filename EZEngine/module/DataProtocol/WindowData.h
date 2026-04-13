#pragma once
#ifndef __D_P_WINDOWDATA_H__
#define __D_P_WINDOWDATA_H__

#include <string>
#include "core/Types.h"
#include "DataProtocol/MathTypes.h"

namespace DataProtocol
{
	enum class WindowMode : EZ::u8
	{
		Windowed = 0,
		BorderlessFullscreen,
		ExclusiveFullscreen
	};

	enum class WindowBackendHint : EZ::u8
	{
		None = 0,
		OpenGL,
		Vulkan,
		Metal
	};

	enum class WindowCreateFlags : EZ::u64
	{
		None = 0,
		Hidden = 1ull << 0,
		Resizable = 1ull << 1,
		Borderless = 1ull << 2,
		HighPixelDensity = 1ull << 3,
		AlwaysOnTop = 1ull << 4,
		Transparent = 1ull << 5,
		MouseGrabbed = 1ull << 6,
		KeyboardGrabbed = 1ull << 7,
		Maximized = 1ull << 8,
		Minimized = 1ull << 9,
		NotFocusable = 1ull << 10
	};

	inline constexpr WindowCreateFlags operator|(WindowCreateFlags lhs, WindowCreateFlags rhs)
	{
		return static_cast<WindowCreateFlags>(
			static_cast<EZ::u64>(lhs) | static_cast<EZ::u64>(rhs)
			);
	}

	inline constexpr WindowCreateFlags operator&(WindowCreateFlags lhs, WindowCreateFlags rhs)
	{
		return static_cast<WindowCreateFlags>(
			static_cast<EZ::u64>(lhs) & static_cast<EZ::u64>(rhs)
			);
	}

	inline constexpr bool HasFlag(WindowCreateFlags value, WindowCreateFlags flag)
	{
		return (static_cast<EZ::u64>(value) & static_cast<EZ::u64>(flag)) != 0;
	}

	struct WindowDesc
	{
		static constexpr const char* TypeName() noexcept { return "WindowDesc"; }
		static constexpr EZ::u32 Version() noexcept { return 1; }

		template<typename Visitor>
		void Reflect(Visitor&& v)
		{
			v("title", title);
			v("position", position);
			v("size", size);
			v("minSize", minSize);
			v("maxSize", maxSize);
			v("mode", mode);
			v("backendHint", backendHint);
			v("flags", flags);
		}

		std::string title = "EZEngine";

		// position.x / position.y < 0 时按居中处理
		IVec2 position{ -1, -1 };

		// 逻辑尺寸（窗口客户区尺寸）
		UVec2 size{ 1280, 720 };
		UVec2 minSize{ 0, 0 };
		UVec2 maxSize{ 0, 0 };

		WindowMode mode = WindowMode::Windowed;
		WindowBackendHint backendHint = WindowBackendHint::None;
		WindowCreateFlags flags = WindowCreateFlags::Resizable;
	};

	enum class WindowEventType : EZ::u16
	{
		None = 0,

		QuitRequested,

		Shown,
		Hidden,
		Exposed,

		Moved,
		Resized,
		PixelSizeChanged,

		Minimized,
		Maximized,
		Restored,

		MouseEnter,
		MouseLeave,
		FocusGained,
		FocusLost,

		CloseRequested,
		Destroyed,

		DisplayChanged,
		EnterFullscreen,
		LeaveFullscreen
	};

	struct WindowEventData
	{
		static constexpr const char* TypeName() noexcept { return "WindowEventData"; }
		static constexpr EZ::u32 Version() noexcept { return 1; }

		template<typename Visitor>
		void Reflect(Visitor&& v)
		{
			v("type", type);
			v("position", position);
			v("size", size);
			v("pixelSize", pixelSize);
			v("backendWindowID", backendWindowID);
		}

		WindowEventType type = WindowEventType::None;
		IVec2 position{};
		UVec2 size{};
		UVec2 pixelSize{};
		EZ::u64 backendWindowID = 0;
	};

	struct NativeWindowHandle
	{
		void* backendWindow = nullptr;
		void* nativeWindow = nullptr;
		void* extraHandle = nullptr;
	};
}

#endif