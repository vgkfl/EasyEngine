#pragma once
#ifndef __D_P_ANIMATOR_CONTROLLER_ASSET_H__
#define __D_P_ANIMATOR_CONTROLLER_ASSET_H__

#include <string>
#include <vector>

#include "core/Types.h"
#include "AnimationClipAsset.h"

namespace DataProtocol
{
	enum class AnimatorParameterType : EZ::u8
	{
		Float = 0,
		Int,
		Bool,
		Trigger
	};

	enum class AnimatorConditionMode : EZ::u8
	{
		If = 0,
		IfNot,
		Greater,
		Less,
		Equals,
		NotEqual,
		Triggered
	};

	struct AnimatorParameterDesc
	{
		static constexpr const char* TypeName() noexcept { return "AnimatorParameterDesc"; }
		static constexpr EZ::u32 Version() noexcept { return 1; }

		template<typename Visitor>
		void Reflect(Visitor&& v)
		{
			v("name", name);
			v("type", type);
			v("defaultFloat", defaultFloat);
			v("defaultInt", defaultInt);
			v("defaultBool", defaultBool);
		}

		std::string name;
		AnimatorParameterType type = AnimatorParameterType::Float;

		EZ::f32 defaultFloat = 0.0f;
		EZ::i32 defaultInt = 0;
		bool defaultBool = false;
	};

	struct AnimatorConditionDesc
	{
		static constexpr const char* TypeName() noexcept { return "AnimatorConditionDesc"; }
		static constexpr EZ::u32 Version() noexcept { return 1; }

		template<typename Visitor>
		void Reflect(Visitor&& v)
		{
			v("parameterName", parameterName);
			v("mode", mode);
			v("thresholdFloat", thresholdFloat);
			v("thresholdInt", thresholdInt);
			v("thresholdBool", thresholdBool);
		}

		std::string parameterName;
		AnimatorConditionMode mode = AnimatorConditionMode::If;
		EZ::f32 thresholdFloat = 0.0f;
		EZ::i32 thresholdInt = 0;
		bool thresholdBool = false;
	};

	struct AnimatorTransitionDesc
	{
		static constexpr const char* TypeName() noexcept { return "AnimatorTransitionDesc"; }
		static constexpr EZ::u32 Version() noexcept { return 1; }

		template<typename Visitor>
		void Reflect(Visitor&& v)
		{
			v("fromState", fromState);
			v("toState", toState);
			v("anyState", anyState);
			v("hasExitTime", hasExitTime);
			v("exitTimeNormalized", exitTimeNormalized);
			v("conditions", conditions);
		}

		std::string fromState;
		std::string toState;

		bool anyState = false;
		bool hasExitTime = false;
		EZ::f32 exitTimeNormalized = 1.0f;

		std::vector<AnimatorConditionDesc> conditions;
	};

	struct AnimatorStateDesc
	{
		static constexpr const char* TypeName() noexcept { return "AnimatorStateDesc"; }
		static constexpr EZ::u32 Version() noexcept { return 1; }

		template<typename Visitor>
		void Reflect(Visitor&& v)
		{
			v("name", name);
			v("clipName", clipName);
			v("looping", looping);
			v("speed", speed);
			v("applyScaleKeys", applyScaleKeys);
		}

		std::string name;
		std::string clipName;

		// 运行时绑定，不参与序列化
		const DataProtocol::AnimationClipAsset* clipAsset = nullptr;

		bool looping = true;
		EZ::f32 speed = 1.0f;
		bool applyScaleKeys = false;
	};

	struct AnimatorControllerAsset
	{
		static constexpr const char* TypeName() noexcept { return "AnimatorControllerAsset"; }
		static constexpr EZ::u32 Version() noexcept { return 1; }

		template<typename Visitor>
		void Reflect(Visitor&& v)
		{
			v("name", name);
			v("sourcePath", sourcePath);
			v("controllerName", controllerName);
			v("defaultState", defaultState);
			v("parameters", parameters);
			v("states", states);
			v("transitions", transitions);
		}

		const AnimatorStateDesc* FindState(const std::string& stateName) const
		{
			for (const auto& state : states)
			{
				if (state.name == stateName)
				{
					return &state;
				}
			}
			return nullptr;
		}

		AnimatorStateDesc* FindState(const std::string& stateName)
		{
			for (auto& state : states)
			{
				if (state.name == stateName)
				{
					return &state;
				}
			}
			return nullptr;
		}

		const AnimatorParameterDesc* FindParameter(const std::string& parameterName) const
		{
			for (const auto& parameter : parameters)
			{
				if (parameter.name == parameterName)
				{
					return &parameter;
				}
			}
			return nullptr;
		}

		std::string name = "";
		std::string sourcePath;
		std::string controllerName;
		std::string defaultState;

		std::vector<AnimatorParameterDesc> parameters;
		std::vector<AnimatorStateDesc> states;
		std::vector<AnimatorTransitionDesc> transitions;
	};
}

#endif