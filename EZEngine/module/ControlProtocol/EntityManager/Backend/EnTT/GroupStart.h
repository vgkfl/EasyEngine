#pragma once
#ifndef __C_P_ENTITYMANAGER_BACKEND_ENTT_GROUPSTART_H__
#define __C_P_ENTITYMANAGER_BACKEND_ENTT_GROUPSTART_H__

#include <entt/entt.hpp>
#include "core/TypeList.h"
#include "ControlProtocol/EntityManager/Backend/EnTT/GroupExpr.h"

namespace IMPL::EntityManager
{
	template<typename... Owned>
	struct GroupStart
	{
		using OwnedTypes = EZ::TypeList<Owned...>;

		template<typename... Get>
		[[nodiscard]] auto Have() const
		{
			return GroupExpr<EZ::TypeList<Owned...>, EZ::TypeList<Get...>, EZ::TypeList<>>{};
		}

		template<typename... Exclude>
		[[nodiscard]] auto NoHave() const
		{
			return GroupExpr<EZ::TypeList<Owned...>, EZ::TypeList<>, EZ::TypeList<Exclude...>>{};
		}

		template<typename Registry>
		[[nodiscard]] auto Build(Registry& registry) const
		{
			return registry.template group<Owned...>();
		}
	};

	template<typename... Owned>
	[[nodiscard]] constexpr auto MakeGroup()
	{
		return GroupStart<Owned...>{};
	}

	template<typename... Owned>
	using Group = GroupStart<Owned...>;
}

#endif
