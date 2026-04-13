#include "ControlProtocol/EntityManager/EntityManager.h"

namespace ControlProtocol
{
	EntityManager::EntityManager() = default;
	EntityManager::~EntityManager() = default;

	EZ::Entity EntityManager::CreateEntity()
	{
		return Detail::FromBackendEntity(m_registry.create());
	}

	bool EntityManager::IsValid(EZ::Entity entity) const
	{
		return m_registry.valid(Detail::ToBackendEntity(entity));
	}

	void EntityManager::DestroyEntity(EZ::Entity entity)
	{
		m_registry.destroy(Detail::ToBackendEntity(entity));
	}
}