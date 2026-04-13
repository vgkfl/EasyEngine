#include "ControlProtocol/AnimationManager/AnimationManager.h"

#include <cmath>
#include <string>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "core/Context/RunTimeContext/ProjectContext.h"
#include "core/Context/RunTimeContext/WorldContext.h"

namespace
{
	static float Clamp01(float v)
	{
		if (v < 0.0f) return 0.0f;
		if (v > 1.0f) return 1.0f;
		return v;
	}

	static DataProtocol::Vec3 LerpVec3(
		const DataProtocol::Vec3& a,
		const DataProtocol::Vec3& b,
		float t)
	{
		return DataProtocol::Vec3
		{
			a.x + (b.x - a.x) * t,
			a.y + (b.y - a.y) * t,
			a.z + (b.z - a.z) * t
		};
	}

	static DataProtocol::Quat NormalizeQuat(const DataProtocol::Quat& q)
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

	static DataProtocol::Quat NlerpQuat(
		const DataProtocol::Quat& a,
		const DataProtocol::Quat& b,
		float t)
	{
		DataProtocol::Quat qb = b;

		float dot = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
		if (dot < 0.0f)
		{
			qb.x = -qb.x;
			qb.y = -qb.y;
			qb.z = -qb.z;
			qb.w = -qb.w;
		}

		DataProtocol::Quat out
		{
			a.x + (qb.x - a.x) * t,
			a.y + (qb.y - a.y) * t,
			a.z + (qb.z - a.z) * t,
			a.w + (qb.w - a.w) * t
		};
		return NormalizeQuat(out);
	}


	template<typename TVecKeyArray>
	static DataProtocol::Vec3 SampleVec3Keys(
		const TVecKeyArray& keys,
		float sampleTime,
		const DataProtocol::Vec3& fallbackValue)
	{
		if (keys.empty())
		{
			return fallbackValue;
		}

		if (keys.size() == 1)
		{
			return keys[0].value;
		}

		if (sampleTime <= keys.front().time)
		{
			return keys.front().value;
		}

		if (sampleTime >= keys.back().time)
		{
			return keys.back().value;
		}

		for (size_t i = 0; i + 1 < keys.size(); ++i)
		{
			const auto& a = keys[i];
			const auto& b = keys[i + 1];

			if (sampleTime >= a.time && sampleTime <= b.time)
			{
				const float dt = b.time - a.time;
				const float t = (dt > 0.000001f)
					? Clamp01((sampleTime - a.time) / dt)
					: 0.0f;
				return LerpVec3(a.value, b.value, t);
			}
		}

		return keys.back().value;
	}

	template<typename TQuatKeyArray>
	static DataProtocol::Quat SampleQuatKeys(
		const TQuatKeyArray& keys,
		float sampleTime,
		const DataProtocol::Quat& fallbackValue)
	{
		if (keys.empty())
		{
			return fallbackValue;
		}

		if (keys.size() == 1)
		{
			return NormalizeQuat(keys[0].value);
		}

		if (sampleTime <= keys.front().time)
		{
			return NormalizeQuat(keys.front().value);
		}

		if (sampleTime >= keys.back().time)
		{
			return NormalizeQuat(keys.back().value);
		}

		for (size_t i = 0; i + 1 < keys.size(); ++i)
		{
			const auto& a = keys[i];
			const auto& b = keys[i + 1];

			if (sampleTime >= a.time && sampleTime <= b.time)
			{
				const float dt = b.time - a.time;
				const float t = (dt > 0.000001f)
					? Clamp01((sampleTime - a.time) / dt)
					: 0.0f;
				return NlerpQuat(a.value, b.value, t);
			}
		}

		return NormalizeQuat(keys.back().value);
	}

	static const DataProtocol::BoneAnimationTrack* FindTrackForBoneIndex(
		const DataProtocol::AnimationClipAsset& clip,
		EZ::u32 boneIndex)
	{
		for (const auto& track : clip.tracks)
		{
			if (track.skeletonBoneIndex == boneIndex)
			{
				return &track;
			}
		}
		return nullptr;
	}
}

namespace ControlProtocol
{
	void AnimationManager::Reset()
	{
		m_GlobalTime = 0.0f;
		m_LocalPoseScratch.clear();
		m_GlobalPoseScratch.clear();
		m_GlobalComputedScratch.clear();
	}

	void AnimationManager::Update(EZ::ProjectContext& project, EZ::WorldContext& world, float deltaTime)
	{
		(void)project;
		(void)world;
		m_GlobalTime += deltaTime;
	}

	float AnimationManager::GetGlobalTime() const
	{
		return m_GlobalTime;
	}

	bool AnimationManager::Evaluate(
		const DataProtocol::SkeletonAsset& skeleton,
		const DataProtocol::SkinAsset& skin,
		const DataProtocol::AnimationClipAsset* clip,
		float sampleTime,
		bool applyScaleKeys,
		BaseProtocol::BonePoseComponent& outPose,
		BaseProtocol::SkinningPaletteComponent& outPalette)
	{
		const size_t boneCount = skeleton.bones.size();
		m_LocalPoseScratch.resize(boneCount);
		m_GlobalPoseScratch.resize(boneCount, glm::mat4(1.0f));
		m_GlobalComputedScratch.assign(boneCount, 0);
		outPose.localPoses.resize(boneCount);
		outPalette.UseOwnedStorage();
		outPalette.finalMatrices.resize(skin.boneBindings.size(), DataProtocol::Mat4{});

		BuildLocalPose(skeleton, clip, sampleTime, applyScaleKeys, outPose);

		for (EZ::u32 boneIndex = 0; boneIndex < static_cast<EZ::u32>(boneCount); ++boneIndex)
		{
			ComputeGlobalPoseRecursive(skeleton, boneIndex);
		}

		for (EZ::u32 paletteIndex = 0; paletteIndex < static_cast<EZ::u32>(skin.boneBindings.size()); ++paletteIndex)
		{
			const auto& binding = skin.boneBindings[paletteIndex];
			if (binding.skeletonBoneIndex < 0 ||
				binding.skeletonBoneIndex >= static_cast<EZ::i32>(m_GlobalPoseScratch.size()))
			{
				outPalette.finalMatrices[paletteIndex] = DataProtocol::Mat4{};
				continue;
			}

			const glm::mat4 finalMatrix =
				m_GlobalPoseScratch[static_cast<EZ::u32>(binding.skeletonBoneIndex)] *
				ToGLM(binding.inverseBindMatrix);
			outPalette.finalMatrices[paletteIndex] = FromGLM(finalMatrix);
		}

		outPose.dirty = false;
		outPalette.dirty = false;
		return true;
	}

	bool ControlProtocol::AnimationManager::SampleBoneLocalTransform(
		const DataProtocol::SkeletonAsset& skeleton,
		const DataProtocol::AnimationClipAsset* clip,
		float sampleTime,
		bool applyScaleKeys,
		EZ::u32 boneIndex,
		DataProtocol::Transform& outLocalTransform)
	{
		if (boneIndex >= static_cast<EZ::u32>(skeleton.bones.size()))
		{
			return false;
		}

		// 这里默认 bone 里有 localBindTransform
		// 如果你工程里字段名不同，改成你自己的 bind pose 本地变换字段
		const auto& bindBone = skeleton.bones[boneIndex];
		outLocalTransform = { bindBone.localBindPosition,bindBone.localBindRotation,bindBone.localBindScale };

		if (!clip)
		{
			return true;
		}

		float wrappedTime = sampleTime;
		if (clip->duration > 0.000001f)
		{
			while (wrappedTime < 0.0f)
			{
				wrappedTime += clip->duration;
			}
			while (wrappedTime >= clip->duration)
			{
				wrappedTime -= clip->duration;
			}
		}

		const DataProtocol::BoneAnimationTrack* track = FindTrackForBoneIndex(*clip, boneIndex);
		if (!track)
		{
			return true;
		}

		outLocalTransform.position = FromGLM(SampleVec3Keys(
			track->positionKeys,
			wrappedTime,
			ToGLM(outLocalTransform.position)));

		outLocalTransform.rotation = FromGLM(SampleQuatKeys(
			track->rotationKeys,
			wrappedTime,
			ToGLM(outLocalTransform.rotation)));

		if (applyScaleKeys)
		{
			outLocalTransform.scale = FromGLM(SampleVec3Keys(
				track->scaleKeys,
				wrappedTime,
				ToGLM(outLocalTransform.scale)));
		}

		return true;
	}

	glm::vec3 AnimationManager::ToGLM(const DataProtocol::Vec3& v)
	{
		return glm::vec3(v.x, v.y, v.z);
	}

	glm::quat AnimationManager::ToGLM(const DataProtocol::Quat& q)
	{
		return glm::quat(q.w, q.x, q.y, q.z);
	}

	glm::mat4 AnimationManager::ToGLM(const DataProtocol::Mat4& m)
	{
		glm::mat4 result(1.0f);
		for (EZ::u32 i = 0; i < 16; ++i)
		{
			result[i / 4][i % 4] = m.m[i];
		}
		return result;
	}

	DataProtocol::Vec3 AnimationManager::FromGLM(const glm::vec3& v)
	{
		DataProtocol::Vec3 result{};
		result.x = v.x;
		result.y = v.y;
		result.z = v.z;
		return result;
	}

	DataProtocol::Quat AnimationManager::FromGLM(const glm::quat& q)
	{
		DataProtocol::Quat result{};
		result.x = q.x;
		result.y = q.y;
		result.z = q.z;
		result.w = q.w;
		return result;
	}

	DataProtocol::Mat4 AnimationManager::FromGLM(const glm::mat4& m)
	{
		DataProtocol::Mat4 result{};
		for (EZ::u32 i = 0; i < 16; ++i)
		{
			result.m[i] = m[i / 4][i % 4];
		}
		return result;
	}

	glm::mat4 AnimationManager::ComposeLocalMatrix(const BaseProtocol::BoneLocalPose& pose)
	{
		const glm::mat4 T = glm::translate(glm::mat4(1.0f), ToGLM(pose.position));
		const glm::mat4 R = glm::mat4_cast(glm::normalize(ToGLM(pose.rotation)));
		const glm::mat4 S = glm::scale(glm::mat4(1.0f), ToGLM(pose.scale));
		return T * R * S;
	}

	float AnimationManager::WrapTime(float timeValue, float duration)
	{
		if (duration <= 0.0f)
		{
			return 0.0f;
		}

		float wrapped = std::fmod(timeValue, duration);
		if (wrapped < 0.0f)
		{
			wrapped += duration;
		}
		return wrapped;
	}

	const DataProtocol::BoneAnimationTrack* AnimationManager::FindTrackForBone(
		const DataProtocol::AnimationClipAsset& clip,
		const DataProtocol::SkeletonAsset& skeleton,
		EZ::u32 boneIndex)
	{
		for (const auto& track : clip.tracks)
		{
			if (track.skeletonBoneIndex == static_cast<EZ::i32>(boneIndex))
			{
				return &track;
			}
		}

		if (boneIndex < skeleton.bones.size())
		{
			const std::string& boneName = skeleton.bones[boneIndex].name;
			for (const auto& track : clip.tracks)
			{
				if (track.boneName == boneName)
				{
					return &track;
				}
			}
		}

		return nullptr;
	}

	glm::vec3 AnimationManager::SampleVec3Keys(
		const std::vector<DataProtocol::AnimationVecKey>& keys,
		float sampleTime,
		const glm::vec3& fallback)
	{
		if (keys.empty())
		{
			return fallback;
		}

		if (keys.size() == 1 || sampleTime <= keys.front().time)
		{
			return ToGLM(keys.front().value);
		}

		if (sampleTime >= keys.back().time)
		{
			return ToGLM(keys.back().value);
		}

		for (size_t i = 0; i + 1 < keys.size(); ++i)
		{
			const auto& a = keys[i];
			const auto& b = keys[i + 1];
			if (sampleTime < a.time || sampleTime > b.time)
			{
				continue;
			}

			const float dt = b.time - a.time;
			if (dt <= 0.000001f)
			{
				return ToGLM(a.value);
			}

			const float t = (sampleTime - a.time) / dt;
			return glm::mix(ToGLM(a.value), ToGLM(b.value), t);
		}

		return ToGLM(keys.back().value);
	}

	glm::quat AnimationManager::SampleQuatKeys(
		const std::vector<DataProtocol::AnimationQuatKey>& keys,
		float sampleTime,
		const glm::quat& fallback)
	{
		if (keys.empty())
		{
			return fallback;
		}

		if (keys.size() == 1 || sampleTime <= keys.front().time)
		{
			return glm::normalize(ToGLM(keys.front().value));
		}

		if (sampleTime >= keys.back().time)
		{
			return glm::normalize(ToGLM(keys.back().value));
		}

		for (size_t i = 0; i + 1 < keys.size(); ++i)
		{
			const auto& a = keys[i];
			const auto& b = keys[i + 1];
			if (sampleTime < a.time || sampleTime > b.time)
			{
				continue;
			}

			const float dt = b.time - a.time;
			if (dt <= 0.000001f)
			{
				return glm::normalize(ToGLM(a.value));
			}

			const float t = (sampleTime - a.time) / dt;
			return glm::normalize(glm::slerp(ToGLM(a.value), ToGLM(b.value), t));
		}

		return glm::normalize(ToGLM(keys.back().value));
	}

	void AnimationManager::BuildLocalPose(
		const DataProtocol::SkeletonAsset& skeleton,
		const DataProtocol::AnimationClipAsset* clip,
		float sampleTime,
		bool applyScaleKeys,
		BaseProtocol::BonePoseComponent& outPose)
	{
		for (EZ::u32 boneIndex = 0; boneIndex < static_cast<EZ::u32>(skeleton.bones.size()); ++boneIndex)
		{
			const auto& bone = skeleton.bones[boneIndex];
			auto& local = m_LocalPoseScratch[boneIndex];

			local.position = bone.localBindPosition;
			local.rotation = bone.localBindRotation;
			local.scale = bone.localBindScale;

			if (clip != nullptr)
			{
				const auto* track = FindTrackForBone(*clip, skeleton, boneIndex);
				if (track != nullptr)
				{
					local.position = FromGLM(
						SampleVec3Keys(track->positionKeys, sampleTime, ToGLM(local.position)));
					local.rotation = FromGLM(
						SampleQuatKeys(track->rotationKeys, sampleTime, ToGLM(local.rotation)));

					if (applyScaleKeys)
					{
						local.scale = FromGLM(
							SampleVec3Keys(track->scaleKeys, sampleTime, ToGLM(local.scale)));
					}
				}
			}

			outPose.localPoses[boneIndex] = local;
		}
	}

	glm::mat4 AnimationManager::ComputeGlobalPoseRecursive(
		const DataProtocol::SkeletonAsset& skeleton,
		EZ::u32 boneIndex)
	{
		if (m_GlobalComputedScratch[boneIndex] != 0)
		{
			return m_GlobalPoseScratch[boneIndex];
		}

		glm::mat4 global = ComposeLocalMatrix(m_LocalPoseScratch[boneIndex]);
		const auto& bone = skeleton.bones[boneIndex];
		if (bone.parentIndex >= 0)
		{
			global = ComputeGlobalPoseRecursive(skeleton, static_cast<EZ::u32>(bone.parentIndex)) * global;
		}

		m_GlobalPoseScratch[boneIndex] = global;
		m_GlobalComputedScratch[boneIndex] = 1;
		return global;
	}

	void ControlProtocol::AnimationManager::RebuildPoseAndPalette(
		const DataProtocol::SkeletonAsset& skeleton,
		const DataProtocol::SkinAsset& skin,
		BaseProtocol::BonePoseComponent& ioPose,
		BaseProtocol::SkinningPaletteComponent& outPalette)
	{
		const size_t boneCount = skeleton.bones.size();

		m_LocalPoseScratch.resize(boneCount);
		m_GlobalPoseScratch.resize(boneCount, glm::mat4(1.0f));
		m_GlobalComputedScratch.assign(boneCount, 0);

		outPalette.finalMatrices.resize(skin.boneBindings.size(), DataProtocol::Mat4{});

		for (EZ::u32 boneIndex = 0; boneIndex < static_cast<EZ::u32>(boneCount); ++boneIndex)
		{
			BaseProtocol::BoneLocalPose local{};

			if (boneIndex < static_cast<EZ::u32>(ioPose.localPoses.size()))
			{
				local = ioPose.localPoses[boneIndex];
			}
			else
			{
				local.position = skeleton.bones[boneIndex].localBindPosition;
				local.rotation = skeleton.bones[boneIndex].localBindRotation;
				local.scale = skeleton.bones[boneIndex].localBindScale;
			}

			m_LocalPoseScratch[boneIndex] = local;
			m_GlobalPoseScratch[boneIndex] = glm::mat4(1.0f);
		}

		for (EZ::u32 boneIndex = 0; boneIndex < static_cast<EZ::u32>(boneCount); ++boneIndex)
		{
			ComputeGlobalPoseRecursive(skeleton, boneIndex);
		}

		for (EZ::u32 paletteIndex = 0; paletteIndex < static_cast<EZ::u32>(skin.boneBindings.size()); ++paletteIndex)
		{
			const auto& binding = skin.boneBindings[paletteIndex];
			if (binding.skeletonBoneIndex < 0 ||
				binding.skeletonBoneIndex >= static_cast<EZ::i32>(m_GlobalPoseScratch.size()))
			{
				outPalette.finalMatrices[paletteIndex] = DataProtocol::Mat4{};
				continue;
			}

			const glm::mat4 finalMatrix =
				m_GlobalPoseScratch[static_cast<EZ::u32>(binding.skeletonBoneIndex)] *
				ToGLM(binding.inverseBindMatrix);

			outPalette.finalMatrices[paletteIndex] = FromGLM(finalMatrix);
		}

		ioPose.dirty = false;
		outPalette.dirty = false;
	}
}