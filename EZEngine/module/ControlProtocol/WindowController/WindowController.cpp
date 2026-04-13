#include "ControlProtocol/WindowController/WindowController.h"
#include "ControlProtocol/WindowController/IMPL/IMPL_WindowController.h"

#include <algorithm>
#include <cstdint>
#include <deque>
#include <string>
#include <unordered_map>
#include <utility>

namespace
{
	struct DispatchTarget
	{
		std::deque<DataProtocol::WindowEventData>* queue = nullptr;
		bool* shouldClose = nullptr;
	};

	struct RawEventListener
	{
		EZ::u64 id = 0;
		ControlProtocol::WindowController::RawBackendEventCallback callback = nullptr;
		void* userData = nullptr;
	};

	struct GlobalWindowDispatch
	{
		std::unordered_map<SDL_WindowID, DispatchTarget> targets;
		std::vector<RawEventListener> rawListeners;
		EZ::u64 nextRawListenerID = 1;
		EZ::u32 liveWindowCount = 0;
		bool autoInitializedVideo = false;
	};

	GlobalWindowDispatch& GetDispatch()
	{
		static GlobalWindowDispatch s_dispatch;
		return s_dispatch;
	}

	bool EnsureVideoSubsystem()
	{
		auto& dispatch = GetDispatch();

		if (SDL_WasInit(SDL_INIT_VIDEO) != 0)
		{
			return true;
		}

		if (!SDL_InitSubSystem(SDL_INIT_VIDEO))
		{
			return false;
		}

		dispatch.autoInitializedVideo = true;
		return true;
	}

	void TryShutdownVideoSubsystem()
	{
		auto& dispatch = GetDispatch();

		if (dispatch.liveWindowCount == 0 && dispatch.autoInitializedVideo)
		{
			SDL_QuitSubSystem(SDL_INIT_VIDEO);
			dispatch.autoInitializedVideo = false;
		}
	}

	int ResolveSDLPosition(EZ::i32 value)
	{
		return (value < 0) ? SDL_WINDOWPOS_CENTERED : static_cast<int>(value);
	}

	SDL_WindowFlags ToSDLWindowFlags(const DataProtocol::WindowDesc& desc)
	{
		SDL_WindowFlags flags = 0;

		if (DataProtocol::HasFlag(desc.flags, DataProtocol::WindowCreateFlags::Hidden))
			flags |= SDL_WINDOW_HIDDEN;

		if (DataProtocol::HasFlag(desc.flags, DataProtocol::WindowCreateFlags::Resizable))
			flags |= SDL_WINDOW_RESIZABLE;

		if (DataProtocol::HasFlag(desc.flags, DataProtocol::WindowCreateFlags::Borderless))
			flags |= SDL_WINDOW_BORDERLESS;

		if (DataProtocol::HasFlag(desc.flags, DataProtocol::WindowCreateFlags::HighPixelDensity))
			flags |= SDL_WINDOW_HIGH_PIXEL_DENSITY;

		if (DataProtocol::HasFlag(desc.flags, DataProtocol::WindowCreateFlags::AlwaysOnTop))
			flags |= SDL_WINDOW_ALWAYS_ON_TOP;

		if (DataProtocol::HasFlag(desc.flags, DataProtocol::WindowCreateFlags::Transparent))
			flags |= SDL_WINDOW_TRANSPARENT;

		if (DataProtocol::HasFlag(desc.flags, DataProtocol::WindowCreateFlags::MouseGrabbed))
			flags |= SDL_WINDOW_MOUSE_GRABBED;

		if (DataProtocol::HasFlag(desc.flags, DataProtocol::WindowCreateFlags::KeyboardGrabbed))
			flags |= SDL_WINDOW_KEYBOARD_GRABBED;

		if (DataProtocol::HasFlag(desc.flags, DataProtocol::WindowCreateFlags::Maximized))
			flags |= SDL_WINDOW_MAXIMIZED;

		if (DataProtocol::HasFlag(desc.flags, DataProtocol::WindowCreateFlags::Minimized))
			flags |= SDL_WINDOW_MINIMIZED;

		if (DataProtocol::HasFlag(desc.flags, DataProtocol::WindowCreateFlags::NotFocusable))
			flags |= SDL_WINDOW_NOT_FOCUSABLE;

		switch (desc.backendHint)
		{
		case DataProtocol::WindowBackendHint::OpenGL:
			flags |= SDL_WINDOW_OPENGL;
			break;
		case DataProtocol::WindowBackendHint::Vulkan:
			flags |= SDL_WINDOW_VULKAN;
			break;
		case DataProtocol::WindowBackendHint::Metal:
			flags |= SDL_WINDOW_METAL;
			break;
		default:
			break;
		}

		return flags;
	}

	bool TranslateSDLWindowEvent(const SDL_Event& sdlEvent, DataProtocol::WindowEventData& outEvent)
	{
		outEvent = {};

		switch (sdlEvent.type)
		{
		case SDL_EVENT_WINDOW_SHOWN:
			outEvent.type = DataProtocol::WindowEventType::Shown;
			break;
		case SDL_EVENT_WINDOW_HIDDEN:
			outEvent.type = DataProtocol::WindowEventType::Hidden;
			break;
		case SDL_EVENT_WINDOW_EXPOSED:
			outEvent.type = DataProtocol::WindowEventType::Exposed;
			break;
		case SDL_EVENT_WINDOW_MOVED:
			outEvent.type = DataProtocol::WindowEventType::Moved;
			outEvent.position = { static_cast<EZ::i32>(sdlEvent.window.data1), static_cast<EZ::i32>(sdlEvent.window.data2) };
			break;
		case SDL_EVENT_WINDOW_RESIZED:
			outEvent.type = DataProtocol::WindowEventType::Resized;
			outEvent.size = { static_cast<EZ::u32>(sdlEvent.window.data1), static_cast<EZ::u32>(sdlEvent.window.data2) };
			break;
		case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
			outEvent.type = DataProtocol::WindowEventType::PixelSizeChanged;
			outEvent.pixelSize = { static_cast<EZ::u32>(sdlEvent.window.data1), static_cast<EZ::u32>(sdlEvent.window.data2) };
			break;
		case SDL_EVENT_WINDOW_MINIMIZED:
			outEvent.type = DataProtocol::WindowEventType::Minimized;
			break;
		case SDL_EVENT_WINDOW_MAXIMIZED:
			outEvent.type = DataProtocol::WindowEventType::Maximized;
			break;
		case SDL_EVENT_WINDOW_RESTORED:
			outEvent.type = DataProtocol::WindowEventType::Restored;
			break;
		case SDL_EVENT_WINDOW_MOUSE_ENTER:
			outEvent.type = DataProtocol::WindowEventType::MouseEnter;
			break;
		case SDL_EVENT_WINDOW_MOUSE_LEAVE:
			outEvent.type = DataProtocol::WindowEventType::MouseLeave;
			break;
		case SDL_EVENT_WINDOW_FOCUS_GAINED:
			outEvent.type = DataProtocol::WindowEventType::FocusGained;
			break;
		case SDL_EVENT_WINDOW_FOCUS_LOST:
			outEvent.type = DataProtocol::WindowEventType::FocusLost;
			break;
		case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
			outEvent.type = DataProtocol::WindowEventType::CloseRequested;
			break;
		case SDL_EVENT_WINDOW_DESTROYED:
			outEvent.type = DataProtocol::WindowEventType::Destroyed;
			break;
		case SDL_EVENT_WINDOW_DISPLAY_CHANGED:
			outEvent.type = DataProtocol::WindowEventType::DisplayChanged;
			break;
		case SDL_EVENT_WINDOW_ENTER_FULLSCREEN:
			outEvent.type = DataProtocol::WindowEventType::EnterFullscreen;
			break;
		case SDL_EVENT_WINDOW_LEAVE_FULLSCREEN:
			outEvent.type = DataProtocol::WindowEventType::LeaveFullscreen;
			break;
		default:
			return false;
		}

		outEvent.backendWindowID = static_cast<EZ::u64>(sdlEvent.window.windowID);
		return true;
	}
}

namespace ControlProtocol
{
	WindowController::WindowController()
		: m_Impl(std::make_unique<IMPL>())
	{
	}

	WindowController::~WindowController()
	{
		Destroy();
	}

	WindowController::WindowController(WindowController&& other) noexcept = default;
	WindowController& WindowController::operator=(WindowController&& other) noexcept = default;

	bool WindowController::Create(const DataProtocol::WindowDesc& desc)
	{
		if (IsValid())
		{
			return false;
		}

		if (!EnsureVideoSubsystem())
		{
			return false;
		}

		SDL_WindowFlags flags = ToSDLWindowFlags(desc);

		SDL_Window* window = SDL_CreateWindow(
			desc.title.c_str(),
			static_cast<int>(desc.size.x),
			static_cast<int>(desc.size.y),
			flags
		);

		if (!window)
		{
			return false;
		}

		m_Impl->window = window;
		m_Impl->windowID = SDL_GetWindowID(window);
		m_Impl->shouldClose = false;
		m_Impl->desc = desc;
		m_Impl->eventQueue.clear();

		auto& dispatch = GetDispatch();
		dispatch.targets[m_Impl->windowID] = { &m_Impl->eventQueue, &m_Impl->shouldClose };
		++dispatch.liveWindowCount;

		// 位置
		SDL_SetWindowPosition(
			m_Impl->window,
			ResolveSDLPosition(desc.position.x),
			ResolveSDLPosition(desc.position.y)
		);

		// 尺寸约束
		if (desc.minSize.x != 0 || desc.minSize.y != 0)
		{
			SDL_SetWindowMinimumSize(
				m_Impl->window,
				static_cast<int>(desc.minSize.x),
				static_cast<int>(desc.minSize.y)
			);
		}

		if (desc.maxSize.x != 0 || desc.maxSize.y != 0)
		{
			SDL_SetWindowMaximumSize(
				m_Impl->window,
				static_cast<int>(desc.maxSize.x),
				static_cast<int>(desc.maxSize.y)
			);
		}

		// 运行时补充状态
		if (DataProtocol::HasFlag(desc.flags, DataProtocol::WindowCreateFlags::AlwaysOnTop))
		{
			SDL_SetWindowAlwaysOnTop(m_Impl->window, true);
		}

		if (DataProtocol::HasFlag(desc.flags, DataProtocol::WindowCreateFlags::MouseGrabbed))
		{
			SDL_SetWindowMouseGrab(m_Impl->window, true);
		}

		if (DataProtocol::HasFlag(desc.flags, DataProtocol::WindowCreateFlags::KeyboardGrabbed))
		{
			SDL_SetWindowKeyboardGrab(m_Impl->window, true);
		}

		if (desc.mode != DataProtocol::WindowMode::Windowed)
		{
			SetWindowMode(desc.mode);
		}

		return true;
	}

	void WindowController::Destroy()
	{
		if (!IsValid())
		{
			return;
		}

		auto& dispatch = GetDispatch();

		dispatch.targets.erase(m_Impl->windowID);

		if (dispatch.liveWindowCount > 0)
		{
			--dispatch.liveWindowCount;
		}

		SDL_DestroyWindow(m_Impl->window);

		m_Impl->window = nullptr;
		m_Impl->windowID = 0;
		m_Impl->shouldClose = true;

		TryShutdownVideoSubsystem();
	}

	bool WindowController::IsValid() const
	{
		return m_Impl && m_Impl->window != nullptr;
	}

	WindowController::RawEventListenerHandle WindowController::AddRawEventListener(
		RawBackendEventCallback callback,
		void* userData)
	{
		if (!callback)
		{
			return {};
		}

		auto& dispatch = GetDispatch();

		RawEventListener listener{};
		listener.id = dispatch.nextRawListenerID++;
		listener.callback = callback;
		listener.userData = userData;

		dispatch.rawListeners.push_back(listener);

		return RawEventListenerHandle{ listener.id };
	}

	bool WindowController::RemoveRawEventListener(RawEventListenerHandle handle)
	{
		if (!handle.IsValid())
		{
			return false;
		}

		auto& dispatch = GetDispatch();
		auto oldSize = dispatch.rawListeners.size();

		dispatch.rawListeners.erase(
			std::remove_if(
				dispatch.rawListeners.begin(),
				dispatch.rawListeners.end(),
				[&](const RawEventListener& listener)
				{
					return listener.id == handle.id;
				}),
			dispatch.rawListeners.end());

		return dispatch.rawListeners.size() != oldSize;
	}

	void WindowController::PumpEvents()
	{
		SDL_Event event{};

		auto& dispatch = GetDispatch();

		while (SDL_PollEvent(&event))
		{
			const auto rawListeners = dispatch.rawListeners;
			for (const RawEventListener& listener : rawListeners)
			{
				if (listener.callback)
				{
					listener.callback(&event, listener.userData);
				}
			}

			if (event.type == SDL_EVENT_QUIT)
			{
				for (auto& [windowID, target] : dispatch.targets)
				{
					if (target.shouldClose)
					{
						*target.shouldClose = true;
					}

					if (target.queue)
					{
						DataProtocol::WindowEventData data{};
						data.type = DataProtocol::WindowEventType::QuitRequested;
						data.backendWindowID = static_cast<EZ::u64>(windowID);
						target.queue->push_back(data);
					}
				}
				continue;
			}

			DataProtocol::WindowEventData data{};
			if (!TranslateSDLWindowEvent(event, data))
			{
				continue;
			}

			auto it = dispatch.targets.find(static_cast<SDL_WindowID>(data.backendWindowID));
			if (it == dispatch.targets.end())
			{
				continue;
			}

			if (it->second.queue)
			{
				it->second.queue->push_back(data);
			}

			if ((data.type == DataProtocol::WindowEventType::CloseRequested ||
				data.type == DataProtocol::WindowEventType::QuitRequested ||
				data.type == DataProtocol::WindowEventType::Destroyed) &&
				it->second.shouldClose)
			{
				*(it->second.shouldClose) = true;
			}
		}
	}

	bool WindowController::PopEvent(DataProtocol::WindowEventData& outEvent)
	{
		if (m_Impl->eventQueue.empty())
		{
			return false;
		}

		outEvent = m_Impl->eventQueue.front();
		m_Impl->eventQueue.pop_front();
		return true;
	}

	bool WindowController::SetTitle(std::string_view title)
	{
		if (!IsValid())
		{
			return false;
		}

		if (!SDL_SetWindowTitle(m_Impl->window, std::string(title).c_str()))
		{
			return false;
		}

		m_Impl->desc.title = std::string(title);
		return true;
	}

	std::string WindowController::GetTitle() const
	{
		if (!IsValid())
		{
			return {};
		}

		return std::string(SDL_GetWindowTitle(m_Impl->window));
	}

	bool WindowController::SetPosition(const DataProtocol::IVec2& position)
	{
		if (!IsValid())
		{
			return false;
		}

		if (!SDL_SetWindowPosition(
			m_Impl->window,
			ResolveSDLPosition(position.x),
			ResolveSDLPosition(position.y)))
		{
			return false;
		}

		m_Impl->desc.position = position;
		return true;
	}

	DataProtocol::IVec2 WindowController::GetPosition() const
	{
		DataProtocol::IVec2 pos{};

		if (!IsValid())
		{
			return pos;
		}

		int x = 0;
		int y = 0;
		if (SDL_GetWindowPosition(m_Impl->window, &x, &y))
		{
			pos.x = static_cast<EZ::i32>(x);
			pos.y = static_cast<EZ::i32>(y);
		}

		return pos;
	}

	bool WindowController::SetSize(const DataProtocol::UVec2& size)
	{
		if (!IsValid() || size.x == 0 || size.y == 0)
		{
			return false;
		}

		if (!SDL_SetWindowSize(
			m_Impl->window,
			static_cast<int>(size.x),
			static_cast<int>(size.y)))
		{
			return false;
		}

		m_Impl->desc.size = size;
		return true;
	}

	DataProtocol::UVec2 WindowController::GetSize() const
	{
		DataProtocol::UVec2 size{};

		if (!IsValid())
		{
			return size;
		}

		int w = 0;
		int h = 0;
		if (SDL_GetWindowSize(m_Impl->window, &w, &h))
		{
			size.x = static_cast<EZ::u32>(w);
			size.y = static_cast<EZ::u32>(h);
		}

		return size;
	}

	DataProtocol::UVec2 WindowController::GetDrawableSize() const
	{
		DataProtocol::UVec2 size{};

		if (!IsValid())
		{
			return size;
		}

		int w = 0;
		int h = 0;
		if (SDL_GetWindowSizeInPixels(m_Impl->window, &w, &h))
		{
			size.x = static_cast<EZ::u32>(w);
			size.y = static_cast<EZ::u32>(h);
		}

		return size;
	}

	bool WindowController::SetMinimumSize(const DataProtocol::UVec2& size)
	{
		if (!IsValid())
		{
			return false;
		}

		if (!SDL_SetWindowMinimumSize(
			m_Impl->window,
			static_cast<int>(size.x),
			static_cast<int>(size.y)))
		{
			return false;
		}

		m_Impl->desc.minSize = size;
		return true;
	}

	bool WindowController::SetMaximumSize(const DataProtocol::UVec2& size)
	{
		if (!IsValid())
		{
			return false;
		}

		if (!SDL_SetWindowMaximumSize(
			m_Impl->window,
			static_cast<int>(size.x),
			static_cast<int>(size.y)))
		{
			return false;
		}

		m_Impl->desc.maxSize = size;
		return true;
	}

	DataProtocol::UVec2 WindowController::GetMinimumSize() const
	{
		DataProtocol::UVec2 size{};

		if (!IsValid())
		{
			return size;
		}

		int w = 0;
		int h = 0;
		if (SDL_GetWindowMinimumSize(m_Impl->window, &w, &h))
		{
			size.x = static_cast<EZ::u32>(w);
			size.y = static_cast<EZ::u32>(h);
		}

		return size;
	}

	DataProtocol::UVec2 WindowController::GetMaximumSize() const
	{
		DataProtocol::UVec2 size{};

		if (!IsValid())
		{
			return size;
		}

		int w = 0;
		int h = 0;
		if (SDL_GetWindowMaximumSize(m_Impl->window, &w, &h))
		{
			size.x = static_cast<EZ::u32>(w);
			size.y = static_cast<EZ::u32>(h);
		}

		return size;
	}

	bool WindowController::SetWindowMode(DataProtocol::WindowMode mode)
	{
		if (!IsValid())
		{
			return false;
		}

		switch (mode)
		{
		case DataProtocol::WindowMode::Windowed:
			if (!SDL_SetWindowFullscreen(m_Impl->window, false))
			{
				return false;
			}
			m_Impl->desc.mode = DataProtocol::WindowMode::Windowed;
			return true;

		case DataProtocol::WindowMode::BorderlessFullscreen:
			SDL_SetWindowFullscreenMode(m_Impl->window, nullptr);
			if (!SDL_SetWindowFullscreen(m_Impl->window, true))
			{
				return false;
			}
			m_Impl->desc.mode = DataProtocol::WindowMode::BorderlessFullscreen;
			return true;

		case DataProtocol::WindowMode::ExclusiveFullscreen:
			// 这里先退化处理。
			// 真正独占全屏需要额外的 DisplayMode 数据描述。
			SDL_SetWindowFullscreenMode(m_Impl->window, nullptr);
			if (!SDL_SetWindowFullscreen(m_Impl->window, true))
			{
				return false;
			}
			m_Impl->desc.mode = DataProtocol::WindowMode::BorderlessFullscreen;
			return true;

		default:
			return false;
		}
	}

	DataProtocol::WindowMode WindowController::GetWindowMode() const
	{
		if (!IsValid())
		{
			return DataProtocol::WindowMode::Windowed;
		}

		const SDL_WindowFlags flags = SDL_GetWindowFlags(m_Impl->window);
		if ((flags & SDL_WINDOW_FULLSCREEN) != 0)
		{
			return m_Impl->desc.mode;
		}

		return DataProtocol::WindowMode::Windowed;
	}

	bool WindowController::Show()
	{
		return IsValid() ? SDL_ShowWindow(m_Impl->window) : false;
	}

	bool WindowController::Hide()
	{
		return IsValid() ? SDL_HideWindow(m_Impl->window) : false;
	}

	bool WindowController::Raise()
	{
		return IsValid() ? SDL_RaiseWindow(m_Impl->window) : false;
	}

	bool WindowController::Minimize()
	{
		return IsValid() ? SDL_MinimizeWindow(m_Impl->window) : false;
	}

	bool WindowController::Maximize()
	{
		return IsValid() ? SDL_MaximizeWindow(m_Impl->window) : false;
	}

	bool WindowController::Restore()
	{
		return IsValid() ? SDL_RestoreWindow(m_Impl->window) : false;
	}

	bool WindowController::SetResizable(bool enabled)
	{
		return IsValid() ? SDL_SetWindowResizable(m_Impl->window, enabled) : false;
	}

	bool WindowController::SetBordered(bool enabled)
	{
		return IsValid() ? SDL_SetWindowBordered(m_Impl->window, enabled) : false;
	}

	bool WindowController::SetAlwaysOnTop(bool enabled)
	{
		return IsValid() ? SDL_SetWindowAlwaysOnTop(m_Impl->window, enabled) : false;
	}

	bool WindowController::SetMouseGrab(bool enabled)
	{
		return IsValid() ? SDL_SetWindowMouseGrab(m_Impl->window, enabled) : false;
	}

	bool WindowController::SetKeyboardGrab(bool enabled)
	{
		return IsValid() ? SDL_SetWindowKeyboardGrab(m_Impl->window, enabled) : false;
	}

	bool WindowController::SetOpacity(EZ::f32 opacity)
	{
		if (!IsValid())
		{
			return false;
		}

		opacity = std::clamp(opacity, 0.0f, 1.0f);
		return SDL_SetWindowOpacity(m_Impl->window, opacity);
	}

	bool WindowController::IsVisible() const
	{
		if (!IsValid())
		{
			return false;
		}

		return (SDL_GetWindowFlags(m_Impl->window) & SDL_WINDOW_HIDDEN) == 0;
	}

	bool WindowController::IsMinimized() const
	{
		if (!IsValid())
		{
			return false;
		}

		return (SDL_GetWindowFlags(m_Impl->window) & SDL_WINDOW_MINIMIZED) != 0;
	}

	bool WindowController::IsMaximized() const
	{
		if (!IsValid())
		{
			return false;
		}

		return (SDL_GetWindowFlags(m_Impl->window) & SDL_WINDOW_MAXIMIZED) != 0;
	}

	bool WindowController::HasFocus() const
	{
		if (!IsValid())
		{
			return false;
		}

		return (SDL_GetWindowFlags(m_Impl->window) & SDL_WINDOW_INPUT_FOCUS) != 0;
	}

	bool WindowController::ShouldClose() const
	{
		return m_Impl->shouldClose;
	}

	void WindowController::ClearCloseFlag()
	{
		m_Impl->shouldClose = false;
	}

	bool WindowController::Sync()
	{
		return IsValid() ? SDL_SyncWindow(m_Impl->window) : false;
	}

	void* WindowController::GetBackendWindowHandle() const
	{
		return IsValid() ? static_cast<void*>(m_Impl->window) : nullptr;
	}

	DataProtocol::NativeWindowHandle WindowController::GetNativeWindowHandle() const
	{
		DataProtocol::NativeWindowHandle handle{};

		if (!IsValid())
		{
			return handle;
		}

		handle.backendWindow = m_Impl->window;

		SDL_PropertiesID props = SDL_GetWindowProperties(m_Impl->window);
		if (props == 0)
		{
			return handle;
		}

#if defined(_WIN32)
		handle.nativeWindow = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
		handle.extraHandle = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_INSTANCE_POINTER, nullptr);

#elif defined(__APPLE__)
		handle.nativeWindow = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, nullptr);

#else
		// 优先尝试 Wayland
		handle.nativeWindow = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, nullptr);
		handle.extraHandle = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, nullptr);

		// Wayland 没拿到，再尝试 X11
		if (handle.nativeWindow == nullptr)
		{
			const Sint64 x11Window = SDL_GetNumberProperty(props, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0);
			handle.nativeWindow = reinterpret_cast<void*>(static_cast<uintptr_t>(x11Window));
			handle.extraHandle = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_X11_DISPLAY_POINTER, nullptr);
		}
#endif

		return handle;
	}

	EZ::u64 WindowController::GetBackendWindowID() const
	{
		return IsValid() ? static_cast<EZ::u64>(m_Impl->windowID) : 0;
	}

	std::string WindowController::GetLastError()
	{
		return std::string(SDL_GetError());
	}
}