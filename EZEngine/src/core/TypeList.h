#pragma once

#ifndef __TYPELIST_H__
#define __TYPELIST_H__

namespace EZ
{
	template<typename... Ts>
	struct TypeList {};

	template<typename List, typename... Ts>
	struct TypeListAppend;

	template<typename... Old, typename... Ts>
	struct TypeListAppend<TypeList<Old...>, Ts...>
	{
		using type = TypeList<Old..., Ts...>;
	};

	template<typename List, typename... Ts>
	using TypeListAppend_t = typename TypeListAppend<List, Ts...>::type;

}

#endif