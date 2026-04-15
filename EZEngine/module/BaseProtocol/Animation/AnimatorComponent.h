#pragma once
#ifndef __B_P_ANIMATOR_COMPONENT_H__
#define __B_P_ANIMATOR_COMPONENT_H__

#include <string>
#include <unordered_map>
#include <unordered_set>

#include "DataProtocol/AnimatorControllerAsset.h"
#include "RootMotion.h"

namespace BaseProtocol
{
	struct AnimatorComponent
	{
		// Unity风格 API

		void BindController(const DataProtocol::AnimatorControllerAsset* newController)
		{
			controller = newController;
			currentClip = nullptr;
			currentStateName.clear();
			requestedStateName.clear();
			currentTime = 0.0f;
			stateElapsedTime = 0.0f;
			stateSpeed = 1.0f;

			floatParams.clear();
			intParams.clear();
			boolParams.clear();
			triggerParams.clear();

			if (!controller)
			{
				return;
			}

			for (const auto& parameter : controller->parameters)
			{
				switch (parameter.type)
				{
				case DataProtocol::AnimatorParameterType::Float:
					floatParams[parameter.name] = parameter.defaultFloat;
					break;
				case DataProtocol::AnimatorParameterType::Int:
					intParams[parameter.name] = parameter.defaultInt;
					break;
				case DataProtocol::AnimatorParameterType::Bool:
				case DataProtocol::AnimatorParameterType::Trigger:
					boolParams[parameter.name] = parameter.defaultBool;
					break;
				default:
					break;
				}
			}

			if (!controller->defaultState.empty())
			{
				Play(controller->defaultState, 0.0f);
			}
		}

		void Play(const std::string& stateName, float normalizedTime = 0.0f)
		{
			requestedStateName = stateName;
			requestedNormalizedTime = normalizedTime;
			hasExplicitStateRequest = true;
			requestRestartCurrentState = true;
			requestedTransitionDuration = 0.0f;
		}

		void CrossFade(const std::string& stateName, float transitionDuration = 0.1f, float normalizedTime = 0.0f)
		{
			requestedStateName = stateName;
			requestedNormalizedTime = normalizedTime;
			hasExplicitStateRequest = true;
			requestRestartCurrentState = true;
			requestedTransitionDuration = transitionDuration;
		}

		void SetFloat(const std::string& parameterName, float value)
		{
			floatParams[parameterName] = value;
		}

		float GetFloat(const std::string& parameterName, float fallback = 0.0f) const
		{
			auto it = floatParams.find(parameterName);
			return (it != floatParams.end()) ? it->second : fallback;
		}

		void SetInteger(const std::string& parameterName, EZ::i32 value)
		{
			intParams[parameterName] = value;
		}

		EZ::i32 GetInteger(const std::string& parameterName, EZ::i32 fallback = 0) const
		{
			auto it = intParams.find(parameterName);
			return (it != intParams.end()) ? it->second : fallback;
		}

		void SetBool(const std::string& parameterName, bool value)
		{
			boolParams[parameterName] = value;
		}

		bool GetBool(const std::string& parameterName, bool fallback = false) const
		{
			auto it = boolParams.find(parameterName);
			return (it != boolParams.end()) ? it->second : fallback;
		}

		void SetTrigger(const std::string& parameterName)
		{
			triggerParams.insert(parameterName);
			boolParams[parameterName] = true;
		}

		void ResetTrigger(const std::string& parameterName)
		{
			triggerParams.erase(parameterName);
			boolParams[parameterName] = false;
		}

		bool HasTrigger(const std::string& parameterName) const
		{
			return triggerParams.find(parameterName) != triggerParams.end();
		}
		
		void ClearRootMotion()
		{
			hasRootMotion = false;
			extractedRootMotion.Reset();

#ifndef NDEBUG

			debugRootMotionPreviousTime = 0.0f;
			debugRootMotionCurrentTime = 0.0f;
			debugRootMotionPreviousPosition = { 0.0f, 0.0f, 0.0f };
			debugRootMotionCurrentPosition = { 0.0f, 0.0f, 0.0f };
#endif
		}
		// 运行时状态
		const DataProtocol::AnimatorControllerAsset* controller = nullptr;
		const DataProtocol::AnimationClipAsset* currentClip = nullptr;

		std::string currentStateName;

		float currentTime = 0.0f;
		float stateElapsedTime = 0.0f;
		float playRate = 1.0f;
		float stateSpeed = 1.0f;

		bool playing = true;
		bool looping = true;
		bool applyBindPoseWhenNoClip = true;
		bool applyScaleKeys = false;

		bool applyRootMotion = false;
		bool rootMotionXZOnly = true;
		bool rootMotionIgnoreY = true;
		bool rootMotionYawOnly = true;

		bool hasRootMotion = false;
		BaseProtocol::RootMotionDelta extractedRootMotion{};

		// 指定的根骨骼
		EZ::u32 rootMotionBoneIndex = 0;

		bool consumeRootMotionInPose = true;
		bool rootMotionReferenceValid = false;
		DataProtocol::Transform rootMotionReferenceLocal{};

#ifndef NDEBUG
		float debugRootMotionPreviousTime = 0.0f;
		float debugRootMotionCurrentTime = 0.0f;

		DataProtocol::Vec3 debugRootMotionPreviousPosition{ 0.0f, 0.0f, 0.0f };
		DataProtocol::Vec3 debugRootMotionCurrentPosition{ 0.0f, 0.0f, 0.0f };
#endif

		// Unity 风格的显式切状态请求
		std::string requestedStateName;
		float requestedNormalizedTime = 0.0f;
		float requestedTransitionDuration = 0.0f;
		bool hasExplicitStateRequest = false;
		bool requestRestartCurrentState = false;

		std::unordered_map<std::string, float> floatParams;
		std::unordered_map<std::string, EZ::i32> intParams;
		std::unordered_map<std::string, bool> boolParams;
		std::unordered_set<std::string> triggerParams;
	};
}

#endif