#pragma once
#ifndef __TRANSFORM_SYSTEM_H__
#define __TRANSFORM_SYSTEM_H__

#include "core/System/ISystem.h"
#include "ControlProtocol/TransformManager/TransformManager.h"

class TransformSystem : public EZ::ISystem
{
public:
	explicit TransformSystem(ControlProtocol::TransformManager& manager);

public:
	const char* GetName() const override { return "TransformSystem"; }

	int Initialize(EZ::ProjectContext& project, EZ::WorldContext& world) override;
	void LateUpdate(EZ::ProjectContext& project, EZ::WorldContext& world, float deltaTime) override;

public:
	ControlProtocol::TransformManager& GetManager() { return m_Manager; }
	const ControlProtocol::TransformManager& GetManager() const { return m_Manager; }

private:
	ControlProtocol::TransformManager& m_Manager;
};

#endif
