#pragma once
#ifndef __CORE_I_CONTEXT_H__
#define __CORE_I_CONTEXT_H__

#include <typeindex>

namespace EZ
{
	class IContext
	{
	public:
		virtual ~IContext() = default;

		virtual void RemoveByType(const std::type_index& type) = 0;
		virtual bool HasByType(const std::type_index& type) const = 0;
		virtual void* GetRawByType(const std::type_index& type) const = 0;
	};
}
#endif