#pragma once
#ifndef __TOOL_GAMEOBJECT_H__
#define __TOOL_GAMEOBJECT_H__

#include "BaseProtocol/Entity/Entity.h"
#include "ControlProtocol/EntityManager/EntityManager.h"
#include "DataProtocol/MathTypes.h"

namespace Tool
{
	class GameObject
	{
	private: 
		EZ::Entity value;
		ControlProtocol::EntityManager* em;

	public:
		DataProtocol::Transform& transform;
		GameObject& gameobject;

		static GameObject CreateGameObject();

		template<typename Component>
		auto& GetComponent();

		template<typename...Components>
		auto& HasComponent();

		template<typename Component>
		auto& AddComponent();
	};
}

#endif