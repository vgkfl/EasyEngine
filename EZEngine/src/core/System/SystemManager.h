#pragma once
#ifndef __CORE_SYSTEM_MANAGER_H__
#define __CORE_SYSTEM_MANAGER_H__

#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include "core/System/ISystem.h"

namespace EZ
{
	class SystemManager
	{
	public:
		template<typename T, typename... Args>
		T* AddSystem(Args&&... args)
		{
			static_assert(std::is_base_of_v<ISystem, T>, "T must derive from ISystem");

			auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
			T* raw = ptr.get();
			m_Systems.emplace_back(std::move(ptr));
			return raw;
		}

		int InitializeAll(ProjectContext& project, WorldContext& world)
		{
			for (auto& system : m_Systems)
			{
				const int ret = system->Initialize(project, world);
				if (ret != 0)
				{
					return ret;
				}
			}
			return 0;
		}

		void ShutdownAll(ProjectContext& project, WorldContext& world)
		{
			for (auto it = m_Systems.rbegin(); it != m_Systems.rend(); ++it)
			{
				(*it)->Shutdown(project, world);
			}
		}

		void BeginFrameAll(ProjectContext& project, WorldContext& world, float deltaTime)
		{
			for (auto& system : m_Systems)
			{
				system->BeginFrame(project, world, deltaTime);
			}
		}

		void UpdateAll(ProjectContext& project, WorldContext& world, float deltaTime)
		{
			for (auto& system : m_Systems)
			{
				system->Update(project, world, deltaTime);
			}
		}

		void FixedUpdateAll(ProjectContext& project, WorldContext& world, float fixedDeltaTime)
		{
			for (auto& system : m_Systems)
			{
				system->FixedUpdate(project, world, fixedDeltaTime);
			}
		}

		void LateUpdateAll(ProjectContext& project, WorldContext& world, float deltaTime)
		{
			for (auto& system : m_Systems)
			{
				system->LateUpdate(project, world, deltaTime);
			}
		}

		void EndFrameAll(ProjectContext& project, WorldContext& world, float deltaTime)
		{
			for (auto& system : m_Systems)
			{
				system->EndFrame(project, world, deltaTime);
			}
		}

		void Clear()
		{
			m_Systems.clear();
		}

	private:
		std::vector<std::unique_ptr<ISystem>> m_Systems;
	};
}
#endif