#pragma once
#ifndef __C_P_ENTITYMANAGER_BACKEND_ENTT_GROUPEXPR_H__
#define __C_P_ENTITYMANAGER_BACKEND_ENTT_GROUPEXPR_H__

#include <entt/entt.hpp>
#include "core/TypeList.h"

namespace IMPL::EntityManager
{
	template<typename OwnedList, typename GetList, typename ExcludeList>
	struct GroupExpr;

	template<typename... Owned, typename... Get, typename... Exclude>
	struct GroupExpr<EZ::TypeList<Owned...>, EZ::TypeList<Get...>, EZ::TypeList<Exclude...>>
	{
		using OwnedTypes = EZ::TypeList<Owned...>;
		using GetTypes = EZ::TypeList<Get...>;
		using ExcludeTypes = EZ::TypeList<Exclude...>;

		template<typename... NewGet>
		[[nodiscard]] auto Have() const
		{
			using NewGetList = EZ::TypeListAppend_t<EZ::TypeList<Get...>, NewGet...>;
			return GroupExpr<EZ::TypeList<Owned...>, NewGetList, EZ::TypeList<Exclude...>>{};
		}

		template<typename... NewExclude>
		[[nodiscard]] auto NoHave() const
		{
			using NewExcludeList = EZ::TypeListAppend_t<EZ::TypeList<Exclude...>, NewExclude...>;
			return GroupExpr<EZ::TypeList<Owned...>, EZ::TypeList<Get...>, NewExcludeList>{};
		}

		template<typename Registry>
		[[nodiscard]] auto Build(Registry& registry) const
		{
			return registry.template group<Owned...>(
				entt::get_t<Get...>{},
				entt::exclude_t<Exclude...>{}
			);
		}
	};
}

#endif
