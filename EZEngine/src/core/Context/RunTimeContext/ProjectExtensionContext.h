#pragma once
#ifndef __PROJECT_EXTENSION_CONTEXT_H__
#define __PROJECT_EXTENSION_CONTEXT_H__

#include <memory>
#include <typeindex>
#include <unordered_map>
#include <utility>

namespace EZ
{
	class ProjectExtensionContext
	{
	public:
		template<typename T, typename... Args>
		T& Emplace(Args&&... args)
		{
			auto ptr = std::make_shared<T>(std::forward<Args>(args)...);
			T& ref = *ptr;
			m_Items[std::type_index(typeid(T))] = std::move(ptr);
			return ref;
		}

		template<typename T>
		T* TryGet()
		{
			auto it = m_Items.find(std::type_index(typeid(T)));
			if (it == m_Items.end())
			{
				return nullptr;
			}
			return static_cast<T*>(it->second.get());
		}

		template<typename T>
		const T* TryGet() const
		{
			auto it = m_Items.find(std::type_index(typeid(T)));
			if (it == m_Items.end())
			{
				return nullptr;
			}
			return static_cast<const T*>(it->second.get());
		}

		template<typename T>
		bool Has() const
		{
			return TryGet<T>() != nullptr;
		}

		void Clear()
		{
			m_Items.clear();
		}

	private:
		std::unordered_map<std::type_index, std::shared_ptr<void>> m_Items;
	};
}

#endif