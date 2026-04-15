#pragma once
#ifndef __B_P_TRANSFORM_HIERARCHY_H__
#define __B_P_TRANSFORM_HIERARCHY_H__

#include <optional>

#include "core/Entity/Entity.h"
#include "core/Types.h"

namespace BaseProtocol
{
	struct TransformHierarchy
	{
		std::optional<EZ::Entity> parent;
		std::optional<EZ::Entity> firstChild;
		std::optional<EZ::Entity> nextSibling;
		std::optional<EZ::Entity> prevSibling;
		EZ::u32 depth = 0;

		bool inheritPosition = true;
		bool inheritRotation = true;
		bool inheritScale = true;

		bool hierarchyDirty = true;
	};
}

#endif