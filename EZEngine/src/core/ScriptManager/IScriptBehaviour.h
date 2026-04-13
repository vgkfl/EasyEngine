#pragma once
#ifndef __CORE_I_SCRIPTBEHAVIOUR_H__
#define __CORE_I_SCRIPTBEHAVIOUR_H__

#include <utility>

#include "core/Context/RunTimeContext/WorldContext.h"
#include "core/Entity/Entity.h"
#include "ControlProtocol/EntityManager/EntityManager.h"
#include "ControlProtocol/WindowController/WindowController.h"

namespace EZ
{
	class IScriptBehaviour
	{
	public:
		virtual ~IScriptBehaviour() = default;

		void Bind(WorldContext* world, EZ::Entity entity)
		{
			m_World = world;
			m_Entity = entity;
		}

		EZ::Entity GetEntity() const { return m_Entity; }
		WorldContext* GetWorld() const { return m_World; }

		template<typename T>
		T* TryGetContext() const
		{
			return m_World ? m_World->TryGet<T>() : nullptr;
		}

		template<typename T>
		T& GetContext() const
		{
			return m_World->Get<T>();
		}

		template<typename Component>
		bool HasComponent() const
		{
			auto* entityManager = TryGetContext<ControlProtocol::EntityManager>();
			return entityManager && entityManager->HasComponent<Component>(m_Entity);
		}

		template<typename Component>
		Component* TryGetComponent() const
		{
			auto* entityManager = TryGetContext<ControlProtocol::EntityManager>();
			return entityManager ? entityManager->TryGetComponent<Component>(m_Entity) : nullptr;
		}

		template<typename Component>
		Component& GetComponent() const
		{
			return GetContext<ControlProtocol::EntityManager>().GetComponent<Component>(m_Entity);
		}

		template<typename Component, typename... Args>
		Component& AddComponent(Args&&... args) const
		{
			return GetContext<ControlProtocol::EntityManager>().AddComponent<Component>(
				m_Entity,
				std::forward<Args>(args)...);
		}

		ControlProtocol::WindowController* GetPrimaryWindow() const
		{
			return TryGetContext<ControlProtocol::WindowController>();
		}

	public:
		virtual void Awake() {}
		virtual void Start() {}
		virtual void Update() {}
		virtual void FixedUpdate() {}
		virtual void LateUpdate() {}

		virtual void OnEnable() {}
		virtual void OnDisable() {}
		virtual void OnDestroy() {}

		virtual void OnParentChanged() {}
		virtual void OnChildAdded() {}
		virtual void OnChildRemoved() {}

	private:
		WorldContext* m_World = nullptr;
		EZ::Entity m_Entity{};
	};
}

#endif