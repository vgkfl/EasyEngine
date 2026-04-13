#pragma once
#ifndef __C_P_WINDOWCONTROLLER_H__
#define __C_P_WINDOWCONTROLLER_H__

#include <memory>
#include <string>
#include <string_view>

#include "DataProtocol/WindowData.h"

namespace ControlProtocol
{
	/// <summary>
	/// 一窗一对象的窗口控制器。
	/// 公开头完全 PIMPL，不暴露 SDL3。
	/// </summary>
	class WindowController
	{
	public:
		WindowController();
		~WindowController();

		WindowController(const WindowController&) = delete;
		WindowController& operator=(const WindowController&) = delete;

		WindowController(WindowController&& other) noexcept;
		WindowController& operator=(WindowController&& other) noexcept;

	public:
		bool Create(const DataProtocol::WindowDesc& desc);
		void Destroy();

		bool IsValid() const;


		using RawBackendEventCallback = void(*)(const void* backendEvent, void* userData);

		struct RawEventListenerHandle
		{
			EZ::u64 id = 0;

			bool IsValid() const
			{
				return id != 0;
			}
		};

		static RawEventListenerHandle AddRawEventListener(
			RawBackendEventCallback callback,
			void* userData = nullptr);

		static bool RemoveRawEventListener(RawEventListenerHandle handle);


		// SDL 事件队列是全局的，所以这里做成静态分发
		static void PumpEvents();

		// 取当前窗口自己的事件
		bool PopEvent(DataProtocol::WindowEventData& outEvent);

		bool SetTitle(std::string_view title);
		std::string GetTitle() const;

		bool SetPosition(const DataProtocol::IVec2& position);
		DataProtocol::IVec2 GetPosition() const;

		bool SetSize(const DataProtocol::UVec2& size);
		DataProtocol::UVec2 GetSize() const;
		DataProtocol::UVec2 GetDrawableSize() const;

		bool SetMinimumSize(const DataProtocol::UVec2& size);
		bool SetMaximumSize(const DataProtocol::UVec2& size);
		DataProtocol::UVec2 GetMinimumSize() const;
		DataProtocol::UVec2 GetMaximumSize() const;

		bool SetWindowMode(DataProtocol::WindowMode mode);
		DataProtocol::WindowMode GetWindowMode() const;

		bool Show();
		bool Hide();
		bool Raise();

		bool Minimize();
		bool Maximize();
		bool Restore();

		bool SetResizable(bool enabled);
		bool SetBordered(bool enabled);
		bool SetAlwaysOnTop(bool enabled);
		bool SetMouseGrab(bool enabled);
		bool SetKeyboardGrab(bool enabled);
		bool SetOpacity(EZ::f32 opacity);

		bool IsVisible() const;
		bool IsMinimized() const;
		bool IsMaximized() const;
		bool HasFocus() const;

		bool ShouldClose() const;
		void ClearCloseFlag();

		bool Sync();

		void* GetBackendWindowHandle() const;
		DataProtocol::NativeWindowHandle GetNativeWindowHandle() const;
		EZ::u64 GetBackendWindowID() const;

		static std::string GetLastError();

	private:
		struct IMPL;
		std::unique_ptr<IMPL> m_Impl;
	};
}

#endif