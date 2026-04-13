#pragma once
#ifndef __C_P_PHYSICS_WORLD_CONTROLLER_H__
#define __C_P_PHYSICS_WORLD_CONTROLLER_H__

#include "DataProtocol/MathTypes.h"

namespace ControlProtocol
{
	class PhysicsWorldController
	{
	public:
		bool Initialize()
		{
			m_Initialized = true;
			return true;
		}

		void Shutdown()
		{
			m_Initialized = false;
			m_LastFixedDeltaTime = 0.0f;
		}

		bool IsInitialized() const
		{
			return m_Initialized;
		}

		void SetGravity(const DataProtocol::Vec3& gravity)
		{
			m_Gravity = gravity;
		}

		const DataProtocol::Vec3& GetGravity() const
		{
			return m_Gravity;
		}

		void StepSimulation(float fixedDeltaTime)
		{
			if (!m_Initialized)
			{
				return;
			}

			m_LastFixedDeltaTime = fixedDeltaTime;
		}

		float GetLastFixedDeltaTime() const
		{
			return m_LastFixedDeltaTime;
		}

	private:
		bool m_Initialized = false;
		float m_LastFixedDeltaTime = 0.0f;
		DataProtocol::Vec3 m_Gravity{ 0.0f, -9.81f, 0.0f };
	};
}

#endif