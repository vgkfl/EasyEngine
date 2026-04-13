#pragma once
#ifndef __C_P_ENTITYMANAGER_H__
#define __C_P_ENTITYMANAGER_H__

#define EZ_ENTITYMANAGER_BACKEND_ENTT

#include "core/Entity/Entity.h"
#include "ControlProtocol/EntityManager/Backend/BackendSelector.h"

namespace ControlProtocol
{
	class EntityManager
	{
	private:
		Detail::EntityManagerRegistry m_registry;

	public:
		EntityManager();
		~EntityManager();

		EZ::Entity CreateEntity();

		bool IsValid(EZ::Entity entity) const;

		void DestroyEntity(EZ::Entity entity);

		template<typename Component>
		auto& GetComponent(EZ::Entity entity);

		template<typename Component>
		Component* TryGetComponent(EZ::Entity entity);

		template<typename Component, typename... Args>
		auto& AddComponent(EZ::Entity entity, Args&&... args);

		template<typename... Components>
		bool HasComponent(EZ::Entity entity) const;

		template<typename Component>
		void RemoveComponent(EZ::Entity entity);

		template<typename... Components, typename Func>
		void ForEach(Func&& func);

		template<typename... Components>
		static auto CreateGroup();

		template<typename Group, typename Func>
		void GroupForEach(const Group& group, Func&& func);
	};
}

#include "ControlProtocol/EntityManager/EntityManager.inl"

#endif