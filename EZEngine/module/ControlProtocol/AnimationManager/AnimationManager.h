#pragma once
#ifndef __C_P_ANIMATION_MANAGER_H__
#define __C_P_ANIMATION_MANAGER_H__

#include <vector>

#include <glm/glm.hpp>

#include "BaseProtocol/Animation/BonePoseComponent.h"
#include "BaseProtocol/Animation/SkinningPaletteComponent.h"
#include "DataProtocol/AnimationClipAsset.h"
#include "DataProtocol/SkeletonAsset.h"
#include "DataProtocol/SkinAsset.h"

namespace EZ
{
	struct ProjectContext;
	struct WorldContext;
}

namespace ControlProtocol
{
	class AnimationManager
	{
	public:
		void Reset();
		void Update(EZ::ProjectContext& project, EZ::WorldContext& world, float deltaTime);
		float GetGlobalTime() const;

		bool Evaluate(
			const DataProtocol::SkeletonAsset& skeleton,
			const DataProtocol::SkinAsset& skin,
			const DataProtocol::AnimationClipAsset* clip,
			float sampleTime,
			bool applyScaleKeys,
			BaseProtocol::BonePoseComponent& outPose,
			BaseProtocol::SkinningPaletteComponent& outPalette);

		bool SampleBoneLocalTransform(
			const DataProtocol::SkeletonAsset& skeleton,
			const DataProtocol::AnimationClipAsset* clip,
			float sampleTime,
			bool applyScaleKeys,
			EZ::u32 boneIndex,
			DataProtocol::Transform& outLocalTransform);

		void RebuildPoseAndPalette(
			const DataProtocol::SkeletonAsset& skeleton,
			const DataProtocol::SkinAsset& skin,
			BaseProtocol::BonePoseComponent& ioPose,
			BaseProtocol::SkinningPaletteComponent& outPalette);

	private:
		static glm::vec3 ToGLM(const DataProtocol::Vec3& v);
		static glm::quat ToGLM(const DataProtocol::Quat& q);
		static glm::mat4 ToGLM(const DataProtocol::Mat4& m);

		static DataProtocol::Vec3 FromGLM(const glm::vec3& v);
		static DataProtocol::Quat FromGLM(const glm::quat& q);
		static DataProtocol::Mat4 FromGLM(const glm::mat4& m);

		static glm::mat4 ComposeLocalMatrix(const BaseProtocol::BoneLocalPose& pose);
		static float WrapTime(float timeValue, float duration);

		static const DataProtocol::BoneAnimationTrack* FindTrackForBone(
			const DataProtocol::AnimationClipAsset& clip,
			const DataProtocol::SkeletonAsset& skeleton,
			EZ::u32 boneIndex);

		static glm::vec3 SampleVec3Keys(
			const std::vector<DataProtocol::AnimationVecKey>& keys,
			float sampleTime,
			const glm::vec3& fallback);

		static glm::quat SampleQuatKeys(
			const std::vector<DataProtocol::AnimationQuatKey>& keys,
			float sampleTime,
			const glm::quat& fallback);

		void BuildLocalPose(
			const DataProtocol::SkeletonAsset& skeleton,
			const DataProtocol::AnimationClipAsset* clip,
			float sampleTime,
			bool applyScaleKeys,
			BaseProtocol::BonePoseComponent& outPose);

		glm::mat4 ComputeGlobalPoseRecursive(
			const DataProtocol::SkeletonAsset& skeleton,
			EZ::u32 boneIndex);

	private:
		float m_GlobalTime = 0.0f;
		std::vector<BaseProtocol::BoneLocalPose> m_LocalPoseScratch;
		std::vector<glm::mat4> m_GlobalPoseScratch;
		std::vector<unsigned char> m_GlobalComputedScratch;
	};
}

#endif