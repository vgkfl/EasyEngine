#pragma once
#ifndef __CORE_TYPE_CONTEXT_H__
#define __CORE_TYPE_CONTEXT_H__

#include <typeindex>
#include <type_traits>
#include <unordered_map>
#include <cassert>

#include "IContext.h"

namespace EZ
{
	class TypeContext : public IContext
	{
	public:
		TypeContext() = default;
		~TypeContext() override = default;

	public:
		template<typename T>
		void Register(T* ptr)
		{
			static_assert(!std::is_void_v<T>, "T must not be void.");
			m_Objects[std::type_index(typeid(T))] = static_cast<void*>(ptr);
		}

		template<typename T>
		void Register(T& ref)
		{
			Register<T>(&ref);
		}

		template<typename Interface, typename Impl>
		void RegisterAs(Impl* ptr)
		{
			static_assert(!std::is_void_v<Interface>, "Interface must not be void.");
			static_assert(!std::is_void_v<Impl>, "Impl must not be void.");
			static_assert(std::is_base_of_v<Interface, Impl>,
				"Impl must derive from Interface.");

			m_Objects[std::type_index(typeid(Interface))] =
				static_cast<void*>(static_cast<Interface*>(ptr));
		}

		template<typename Interface, typename Impl>
		void RegisterAs(Impl& ref)
		{
			RegisterAs<Interface>(&ref);
		}

		template<typename T>
		T* TryGet() const
		{
			auto it = m_Objects.find(std::type_index(typeid(T)));
			if (it == m_Objects.end())
			{
				return nullptr;
			}
			return static_cast<T*>(it->second);
		}

		template<typename T>
		T& Get() const
		{
			T* ptr = TryGet<T>();
			assert(ptr && "TypeContext::Get<T>() failed: type not registered.");
			return *ptr;
		}

		template<typename T>
		bool Has() const
		{
			return HasByType(std::type_index(typeid(T)));
		}

		template<typename T>
		void Remove()
		{
			RemoveByType(std::type_index(typeid(T)));
		}

	public:
		void RemoveByType(const std::type_index& type) override
		{
			m_Objects.erase(type);
		}

		bool HasByType(const std::type_index& type) const override
		{
			return m_Objects.find(type) != m_Objects.end();
		}

		void* GetRawByType(const std::type_index& type) const override
		{
			auto it = m_Objects.find(type);
			if (it == m_Objects.end())
			{
				return nullptr;
			}
			return it->second;
		}

		void Clear()
		{
			m_Objects.clear();
		}

	private:
		std::unordered_map<std::type_index, void*> m_Objects;
	};
}

#endif