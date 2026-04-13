#pragma once
#ifndef __C_P_ENTITYMANAGER_BACKEND_ENTT_H__
#define __C_P_ENTITYMANAGER_BACKEND_ENTT_H__

#include <cstdint>
#include <entt/entt.hpp>
#include "core/Entity/Entity.h"

namespace ControlProtocol::Detail
{
	using EntityManagerRegistry = entt::registry;

	inline entt::entity ToBackendEntity(EZ::Entity entity) noexcept
	{
		return static_cast<entt::entity>(static_cast<std::uint32_t>(entity));
	}

	inline EZ::Entity FromBackendEntity(entt::entity entity) noexcept
	{
		return static_cast<EZ::Entity>(static_cast<std::uint32_t>(entity));
	}
}

#endif