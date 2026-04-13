#include "AnimatorSystem.h"

#include <cmath>
#include <vector>

#include "BaseProtocol/Animation/AnimatorComponent.h"
#include "BaseProtocol/Animation/BonePoseComponent.h"
#include "BaseProtocol/Animation/SkinningPaletteComponent.h"
#include "BaseProtocol/Mesh/SkinnedMeshRendererComponent.h"
#include "ControlProtocol/AnimationManager/AnimationManager.h"
#include "ControlProtocol/EntityManager/EntityManager.h"
#include "DataProtocol/AnimatorControllerAsset.h"
#include "core/Context/RunTimeContext/ProjectContext.h"
#include "core/Context/RunTimeContext/WorldContext.h"

namespace
{
	using namespace DataProtocol;

	const AnimatorStateDesc* FindCurrentState(
		const BaseProtocol::AnimatorComponent& animator)
	{
		if (!animator.controller || animator.currentStateName.empty())
		{
			return nullptr;
		}
		return animator.controller->FindState(animator.currentStateName);
	}

	bool EvaluateCondition(
		const BaseProtocol::AnimatorComponent& animator,
		const AnimatorConditionDesc& condition)
	{
		switch (condition.mode)
		{
		case AnimatorConditionMode::If:
			return animator.GetBool(condition.parameterName, false) == true;

		case AnimatorConditionMode::IfNot:
			return animator.GetBool(condition.parameterName, false) == false;

		case AnimatorConditionMode::Greater:
			return animator.GetFloat(condition.parameterName, 0.0f) > condition.thresholdFloat;

		case AnimatorConditionMode::Less:
			return animator.GetFloat(condition.parameterName, 0.0f) < condition.thresholdFloat;

		case AnimatorConditionMode::Equals:
			return animator.GetInteger(condition.parameterName, 0) == condition.thresholdInt;

		case AnimatorConditionMode::NotEqual:
			return animator.GetInteger(condition.parameterName, 0) != condition.thresholdInt;

		case AnimatorConditionMode::Triggered:
			return animator.HasTrigger(condition.parameterName);

		default:
			return false;
		}
	}

	bool AreConditionsSatisfied(
		const BaseProtocol::AnimatorComponent& animator,
		const AnimatorTransitionDesc& transition)
	{
		for (const auto& condition : transition.conditions)
		{
			if (!EvaluateCondition(animator, condition))
			{
				return false;
			}
		}
		return true;
	}

	void ConsumeTransitionTriggers(
		BaseProtocol::AnimatorComponent& animator,
		const AnimatorTransitionDesc& transition)
	{
		for (const auto& condition : transition.conditions)
		{
			if (condition.mode == AnimatorConditionMode::Triggered)
			{
				animator.ResetTrigger(condition.parameterName);
			}
		}
	}

	void EnterState(
		BaseProtocol::AnimatorComponent& animator,
		const AnimatorStateDesc& state,
		float normalizedTime)
	{
		animator.currentStateName = state.name;
		animator.currentClip = state.clipAsset;
		animator.looping = state.looping;
		animator.applyScaleKeys = state.applyScaleKeys;
		animator.stateSpeed = state.speed;

		animator.stateElapsedTime = 0.0f;
		animator.currentTime = 0.0f;

		if (state.clipAsset && state.clipAsset->duration > 0.0f)
		{
			const float clamped = (normalizedTime < 0.0f) ? 0.0f : normalizedTime;
			animator.currentTime = clamped * state.clipAsset->duration;

			if (animator.looping)
			{
				animator.currentTime = std::fmod(animator.currentTime, state.clipAsset->duration);
				if (animator.currentTime < 0.0f)
				{
					animator.currentTime += state.clipAsset->duration;
				}
			}
			else if (animator.currentTime > state.clipAsset->duration)
			{
				animator.currentTime = state.clipAsset->duration;
			}
		}

		animator.playing = true;
		animator.rootMotionReferenceValid = false;
	}

	void ResolveExplicitStateRequest(BaseProtocol::AnimatorComponent& animator)
	{
		if (!animator.controller || !animator.hasExplicitStateRequest)
		{
			return;
		}

		const AnimatorStateDesc* state = animator.controller->FindState(animator.requestedStateName);
		if (state)
		{
			EnterState(animator, *state, animator.requestedNormalizedTime);
		}

		animator.requestedStateName.clear();
		animator.requestedNormalizedTime = 0.0f;
		animator.requestedTransitionDuration = 0.0f;
		animator.hasExplicitStateRequest = false;
		animator.requestRestartCurrentState = false;
	}

	bool TryTakeTransition(BaseProtocol::AnimatorComponent& animator)
	{
		if (!animator.controller)
		{
			return false;
		}

		const float duration = (animator.currentClip && animator.currentClip->duration > 0.0f)
			? animator.currentClip->duration
			: 0.0f;

		const float normalizedStateTime = (duration > 0.0f)
			? (animator.stateElapsedTime / duration)
			: 0.0f;

		for (const auto& transition : animator.controller->transitions)
		{
			if (!transition.anyState)
			{
				if (transition.fromState != animator.currentStateName)
				{
					continue;
				}
			}

			if (transition.hasExitTime && normalizedStateTime < transition.exitTimeNormalized)
			{
				continue;
			}

			if (!AreConditionsSatisfied(animator, transition))
			{
				continue;
			}

			const AnimatorStateDesc* targetState = animator.controller->FindState(transition.toState);
			if (!targetState)
			{
				continue;
			}

			ConsumeTransitionTriggers(animator, transition);
			EnterState(animator, *targetState, 0.0f);
			return true;
		}

		return false;
	}

	void AdvanceAnimatorTime(BaseProtocol::AnimatorComponent& animator, float deltaTime)
	{
		if (!animator.playing || !animator.currentClip)
		{
			return;
		}

		const float duration = animator.currentClip->duration;
		if (duration <= 0.0f)
		{
			animator.currentTime = 0.0f;
			animator.stateElapsedTime = 0.0f;
			return;
		}

		const float delta = deltaTime * animator.playRate * animator.stateSpeed;
		animator.currentTime += delta;
		animator.stateElapsedTime += delta;

		if (animator.looping)
		{
			animator.currentTime = std::fmod(animator.currentTime, duration);
			if (animator.currentTime < 0.0f)
			{
				animator.currentTime += duration;
			}
		}
		else
		{
			if (animator.currentTime < 0.0f)
			{
				animator.currentTime = 0.0f;
			}
			if (animator.currentTime > duration)
			{
				animator.currentTime = duration;
				animator.playing = false;
			}
		}
	}

	EZ::u32 QuantizeSampleTime(float timeValue)
	{
		constexpr float kSampleRate = 120.0f;

		if (timeValue <= 0.0f)
		{
			return 0;
		}

		const float quantized = std::floor(timeValue * kSampleRate + 0.5f);
		if (quantized <= 0.0f)
		{
			return 0;
		}

		return static_cast<EZ::u32>(quantized);
	}

	static DataProtocol::Quat NormalizeQuatRM(const DataProtocol::Quat& q)
	{
		const float lenSq = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
		if (lenSq <= 0.000001f)
		{
			return DataProtocol::Quat{ 0.0f, 0.0f, 0.0f, 1.0f };
		}

		const float invLen = 1.0f / std::sqrt(lenSq);
		return DataProtocol::Quat
		{
			q.x * invLen,
			q.y * invLen,
			q.z * invLen,
			q.w * invLen
		};
	}

	static DataProtocol::Quat InverseQuatRM(const DataProtocol::Quat& q)
	{
		const DataProtocol::Quat n = NormalizeQuatRM(q);
		return DataProtocol::Quat{ -n.x, -n.y, -n.z, n.w };
	}

	static DataProtocol::Quat MulQuatRM(
		const DataProtocol::Quat& a,
		const DataProtocol::Quat& b)
	{
		return DataProtocol::Quat
		{
			a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
			a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
			a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
			a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z
		};
	}

	static bool ExtractRootMotionDelta(
		ControlProtocol::AnimationManager& animationManager,
		const DataProtocol::SkeletonAsset& skeleton,
		const DataProtocol::AnimationClipAsset* clip,
		float previousTime,
		float currentTime,
		bool applyScaleKeys,
		BaseProtocol::AnimatorComponent& animator)
	{
		animator.ClearRootMotion();

		if (!clip)
		{
			return false;
		}

		DataProtocol::Transform prevRoot{};
		DataProtocol::Transform currRoot{};

		if (!animationManager.SampleBoneLocalTransform(
			skeleton,
			clip,
			previousTime,
			applyScaleKeys,
			animator.rootMotionBoneIndex,
			prevRoot))
		{
			return false;
		}

		if (!animationManager.SampleBoneLocalTransform(
			skeleton,
			clip,
			currentTime,
			applyScaleKeys,
			animator.rootMotionBoneIndex,
			currRoot))
		{
			return false;
		}

#ifndef NDEBUG

		animator.debugRootMotionPreviousTime = previousTime;
		animator.debugRootMotionCurrentTime = currentTime;
		animator.debugRootMotionPreviousPosition = prevRoot.position;
		animator.debugRootMotionCurrentPosition = currRoot.position;
#endif

		BaseProtocol::RootMotionDelta delta{};

		const bool wrapped =
			animator.looping &&
			clip->duration > 0.000001f &&
			currentTime < previousTime;

		if (!wrapped)
		{
			delta.deltaPosition =
			{
				currRoot.position.x - prevRoot.position.x,
				currRoot.position.y - prevRoot.position.y,
				currRoot.position.z - prevRoot.position.z
			};

			delta.deltaRotation = MulQuatRM(
				currRoot.rotation,
				InverseQuatRM(prevRoot.rotation));
		}
		else
		{
			// 关键：跨过循环边界时，把位移拆成两段：
			// previous -> clipEnd
			// clipStart -> current
			const float endSampleTime = (clip->duration > 0.0001f)
				? (clip->duration - 0.0001f)
				: 0.0f;

			DataProtocol::Transform startRoot{};
			DataProtocol::Transform endRoot{};

			if (!animationManager.SampleBoneLocalTransform(
				skeleton,
				clip,
				0.0f,
				applyScaleKeys,
				animator.rootMotionBoneIndex,
				startRoot))
			{
				return false;
			}

			if (!animationManager.SampleBoneLocalTransform(
				skeleton,
				clip,
				endSampleTime,
				applyScaleKeys,
				animator.rootMotionBoneIndex,
				endRoot))
			{
				return false;
			}

			const DataProtocol::Vec3 deltaA
			{
				endRoot.position.x - prevRoot.position.x,
				endRoot.position.y - prevRoot.position.y,
				endRoot.position.z - prevRoot.position.z
			};

			const DataProtocol::Vec3 deltaB
			{
				currRoot.position.x - startRoot.position.x,
				currRoot.position.y - startRoot.position.y,
				currRoot.position.z - startRoot.position.z
			};

			delta.deltaPosition =
			{
				deltaA.x + deltaB.x,
				deltaA.y + deltaB.y,
				deltaA.z + deltaB.z
			};

			// 你现在还没消费旋转，这里先给一个保守稳定版本
			delta.deltaRotation = DataProtocol::Quat{ 0.0f, 0.0f, 0.0f, 1.0f };
		}

		if (animator.rootMotionIgnoreY || animator.rootMotionXZOnly)
		{
			delta.deltaPosition.y = 0.0f;
		}

		animator.extractedRootMotion = delta;
		animator.hasRootMotion = true;
		return true;
	}

	static void ConsumeRootMotionFromVisualPose(
		ControlProtocol::AnimationManager& animationManager,
		const DataProtocol::SkeletonAsset& skeleton,
		const DataProtocol::SkinAsset& skin,
		BaseProtocol::AnimatorComponent& animator,
		BaseProtocol::BonePoseComponent& pose,
		BaseProtocol::SkinningPaletteComponent& palette)
	{
		if (!animator.applyRootMotion || !animator.consumeRootMotionInPose)
		{
			return;
		}

		const EZ::u32 boneIndex = animator.rootMotionBoneIndex;
		if (boneIndex >= static_cast<EZ::u32>(pose.localPoses.size()))
		{
			return;
		}

		// 第一次进入当前 state 时，记录参考局部姿态
		if (!animator.rootMotionReferenceValid)
		{
			DataProtocol::Transform refLocal{};
			if (animationManager.SampleBoneLocalTransform(
				skeleton,
				animator.currentClip,
				0.0f,
				animator.applyScaleKeys,
				boneIndex,
				refLocal))
			{
				animator.rootMotionReferenceLocal = refLocal;
				animator.rootMotionReferenceValid = true;
			}
		}

		if (!animator.rootMotionReferenceValid)
		{
			return;
		}

		auto& rootLocal = pose.localPoses[boneIndex];

		// 把已经消费到实体上的位移，从可视姿态里扣掉
		rootLocal.position.x = animator.rootMotionReferenceLocal.position.x;
		rootLocal.position.z = animator.rootMotionReferenceLocal.position.z;

		// 你当前是平面角色，先不消费 Y
		if (!animator.rootMotionIgnoreY && !animator.rootMotionXZOnly)
		{
			rootLocal.position.y = animator.rootMotionReferenceLocal.position.y;
		}

		animationManager.RebuildPoseAndPalette(
			skeleton,
			skin,
			pose,
			palette);
	}
}

void AnimatorSystem::Update(EZ::ProjectContext& project, EZ::WorldContext& world, float deltaTime)
{
	auto* animationManager = world.TryGet<ControlProtocol::AnimationManager>();
	auto* entityManager = world.TryGet<ControlProtocol::EntityManager>();
	if (!animationManager || !entityManager)
	{
		return;
	}

	animationManager->Update(project, world, deltaTime);
	m_SharedPaletteCache.clear();

	entityManager->ForEach<
		BaseProtocol::SkinnedMeshRendererComponent,
		BaseProtocol::AnimatorComponent
	>(
		[&](EZ::Entity entity,
			BaseProtocol::SkinnedMeshRendererComponent& renderer,
			BaseProtocol::AnimatorComponent& animator)
		{
			if (!renderer.meshAsset || !renderer.skeletonAsset || !renderer.skinAsset)
			{
				return;
			}

			if (animator.controller && animator.currentStateName.empty() && !animator.controller->defaultState.empty())
			{
				animator.Play(animator.controller->defaultState, 0.0f);
			}

			ResolveExplicitStateRequest(animator);
			(void)TryTakeTransition(animator);

			const float previousTime = animator.currentTime;
			AdvanceAnimatorTime(animator, deltaTime);
			const float currentTime = animator.currentTime;

			animator.ClearRootMotion();

			if (animator.applyRootMotion &&
				renderer.skeletonAsset &&
				animator.currentClip)
			{
				ExtractRootMotionDelta(
					*animationManager,
					*renderer.skeletonAsset,
					animator.currentClip,
					previousTime,
					currentTime,
					animator.applyScaleKeys,
					animator);
			}

			auto* pose = entityManager->TryGetComponent<BaseProtocol::BonePoseComponent>(entity);
			if (!pose)
			{
				pose = &entityManager->AddComponent<BaseProtocol::BonePoseComponent>(entity);
			}

			auto* palette = entityManager->TryGetComponent<BaseProtocol::SkinningPaletteComponent>(entity);
			if (!palette)
			{
				palette = &entityManager->AddComponent<BaseProtocol::SkinningPaletteComponent>(entity);
			}

			const DataProtocol::AnimationClipAsset* clipToEvaluate = animator.currentClip;
			if (!clipToEvaluate && !animator.applyBindPoseWhenNoClip)
			{
				animator.ClearRootMotion();
				palette->Clear();
				return;
			}

			SharedPaletteKey sharedKey{};
			sharedKey.skeletonAsset = renderer.skeletonAsset;
			sharedKey.skinAsset = renderer.skinAsset;
			sharedKey.clipAsset = clipToEvaluate;
			sharedKey.applyScaleKeys = animator.applyScaleKeys;
			sharedKey.quantizedSampleTick = QuantizeSampleTime(animator.currentTime);
			sharedKey.applyRootMotion = animator.applyRootMotion;
			sharedKey.consumeRootMotionInPose = animator.consumeRootMotionInPose;
			sharedKey.rootMotionBoneIndex = animator.rootMotionBoneIndex;

			auto sharedIt = m_SharedPaletteCache.find(sharedKey);
			if (sharedIt != m_SharedPaletteCache.end())
			{
				palette->UseSharedMatrices(&sharedIt->second);
				palette->dirty = false;
				return;
			}

			animationManager->Evaluate(
				*renderer.skeletonAsset,
				*renderer.skinAsset,
				clipToEvaluate,
				animator.currentTime,
				animator.applyScaleKeys,
				*pose,
				*palette);

			ConsumeRootMotionFromVisualPose(
				*animationManager,
				*renderer.skeletonAsset,
				*renderer.skinAsset,
				animator,
				*pose,
				*palette);

			auto [insertedIt, inserted] = m_SharedPaletteCache.emplace(sharedKey, palette->finalMatrices);
			(void)inserted;

			palette->UseSharedMatrices(&insertedIt->second);
			palette->dirty = false;
		});
}