#pragma once

#include <utility>
#include "ControlProtocol/EntityManager/Backend/EnTT/GroupStart.h"
#include <optional>

namespace ControlProtocol::Detail
{
	using EntityManagerRegistry = entt::registry;

	template<typename... Owned, typename... Get, typename... Exclude, typename GroupType, typename Func>
	static void CallGroupFunc(
		const IMPL::EntityManager::GroupExpr<EZ::TypeList<Owned...>, EZ::TypeList<Get...>, EZ::TypeList<Exclude...>>&,
		GroupType& group,
		entt::entity entity,
		Func&& func)
	{
		func(
			FromBackendEntity(entity),
			group.template get<Owned>(entity)...,
			group.template get<Get>(entity)...
		);
	}

	template<typename... Owned, typename GroupType, typename Func>
	static void CallGroupFunc(
		const IMPL::EntityManager::GroupStart<Owned...>&,
		GroupType& group,
		entt::entity entity,
		Func&& func)
	{
		func(
			FromBackendEntity(entity),
			group.template get<Owned>(entity)...
		);
	}
}

template<typename Component>
auto& ControlProtocol::EntityManager::GetComponent(EZ::Entity entity)
{
	return m_registry.template get<Component>(ControlProtocol::Detail::ToBackendEntity(entity));
}

template<typename Component>
Component* ControlProtocol::EntityManager::TryGetComponent(EZ::Entity entity)
{
	auto e = ControlProtocol::Detail::ToBackendEntity(entity);
	return m_registry.try_get<Component>(e);
}

template<typename Component, typename... Args>
auto& ControlProtocol::EntityManager::AddComponent(EZ::Entity entity, Args&&... args)
{
	return m_registry.template emplace<Component>(
		ControlProtocol::Detail::ToBackendEntity(entity),
		std::forward<Args>(args)...
	);
}

template<typename... Components>
bool ControlProtocol::EntityManager::HasComponent(EZ::Entity entity) const
{
	return m_registry.template all_of<Components...>(ControlProtocol::Detail::ToBackendEntity(entity));
}

template<typename Component>
void ControlProtocol::EntityManager::RemoveComponent(EZ::Entity entity)
{
	m_registry.template remove<Component>(ControlProtocol::Detail::ToBackendEntity(entity));
}

template<typename... Components, typename Func>
void ControlProtocol::EntityManager::ForEach(Func&& func)
{
	auto view = m_registry.template view<Components...>();

	for (auto entity : view)
	{
		func(
			ControlProtocol::Detail::FromBackendEntity(entity),
			view.template get<Components>(entity)...
		);
	}
}

template<typename... Components>
auto ControlProtocol::EntityManager::CreateGroup()
{
	return IMPL::EntityManager::GroupStart<Components...>{};
}

template<typename Group, typename Func>
void ControlProtocol::EntityManager::GroupForEach(const Group& query, Func&& func)
{
	auto group = query.Build(m_registry);

	for (auto entity : group)
	{
		ControlProtocol::Detail::CallGroupFunc(query, group, entity, std::forward<Func>(func));
	}
}
