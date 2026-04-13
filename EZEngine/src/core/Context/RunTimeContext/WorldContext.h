#pragma once
#ifndef __WORLD_CONTEXT_H__
#define __WORLD_CONTEXT_H__

#include <cstddef>
#include <memory>
#include <vector>

#include "core/Context/TypeContext.h"
#include "ControlProtocol/WindowController/WindowController.h"
#include "DataProtocol/WindowData.h"

namespace EZ
{
	struct WorldContext : public TypeContext
	{
	public:
		using WindowPtr = std::unique_ptr<ControlProtocol::WindowController>;
		using WindowContainer = std::vector<WindowPtr>;

	public:
		ControlProtocol::WindowController* CreateWindow(const DataProtocol::WindowDesc& desc)
		{
			auto window = std::make_unique<ControlProtocol::WindowController>();
			if (!window->Create(desc))
			{
				return nullptr;
			}

			ControlProtocol::WindowController* raw = window.get();
			m_Windows.emplace_back(std::move(window));
			RefreshPrimaryWindowRegistration();
			return raw;
		}

		bool DestroyWindow(ControlProtocol::WindowController* window)
		{
			if (!window)
			{
				return false;
			}

			for (auto it = m_Windows.begin(); it != m_Windows.end(); ++it)
			{
				if (it->get() != window)
				{
					continue;
				}

				(*it)->Destroy();
				m_Windows.erase(it);
				RefreshPrimaryWindowRegistration();
				return true;
			}

			return false;
		}

		void DestroyAllWindows()
		{
			for (auto& window : m_Windows)
			{
				if (window)
				{
					window->Destroy();
				}
			}
			m_Windows.clear();
			RefreshPrimaryWindowRegistration();
		}

		void PumpWindowEvents() const
		{
			ControlProtocol::WindowController::PumpEvents();
		}

		bool HasOpenWindow() const
		{
			for (const auto& window : m_Windows)
			{
				if (window && window->IsValid() && !window->ShouldClose())
				{
					return true;
				}
			}
			return false;
		}

		std::size_t GetWindowCount() const
		{
			return m_Windows.size();
		}

		ControlProtocol::WindowController* GetPrimaryWindow()
		{
			return m_Windows.empty() ? nullptr : m_Windows.front().get();
		}

		const ControlProtocol::WindowController* GetPrimaryWindow() const
		{
			return m_Windows.empty() ? nullptr : m_Windows.front().get();
		}

		WindowContainer& GetWindows()
		{
			return m_Windows;
		}

		const WindowContainer& GetWindows() const
		{
			return m_Windows;
		}

		void Clear()
		{
			DestroyAllWindows();
			TypeContext::Clear();
		}

	private:
		void RefreshPrimaryWindowRegistration()
		{
			if (!m_Windows.empty() && m_Windows.front())
			{
				Register<ControlProtocol::WindowController>(m_Windows.front().get());
			}
			else
			{
				Remove<ControlProtocol::WindowController>();
			}
		}

	private:
		WindowContainer m_Windows;
	};
}
#endif
