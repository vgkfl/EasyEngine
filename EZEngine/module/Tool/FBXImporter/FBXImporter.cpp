#include "FBXImporter.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "stb_image/stb_image.h"
#include "ufbx/ufbx.h"

namespace Tool
{
	namespace fs = std::filesystem;

	namespace
	{
		struct PackedStaticVertex
		{
			float px, py, pz;
			float nx, ny, nz;
			float u, v;
		};

		struct PackedSkinnedVertex
		{
			float px, py, pz;
			float nx, ny, nz;
			float u, v;
			EZ::i32 bi[4];
			float bw[4];
		};

		struct SceneDeleter
		{
			void operator()(ufbx_scene* scene) const
			{
				if (scene) {
					ufbx_free_scene(scene);
				}
			}
		};

		using ScenePtr = std::unique_ptr<ufbx_scene, SceneDeleter>;

		struct TempBoneBinding
		{
			bool valid = false;
			std::string boneName;
			EZ::i32 skeletonBoneIndex = -1;
			DataProtocol::Mat4 inverseBindMatrix{};
		};

		static std::string ToString(ufbx_string s)
		{
			return std::string(s.data, s.length);
		}

		static DataProtocol::Vec2 ToDPVec2(const ufbx_vec2& v)
		{
			DataProtocol::Vec2 out{};
			out.x = (EZ::f32)v.x;
			out.y = (EZ::f32)v.y;
			return out;
		}

		static DataProtocol::Vec3 ToDPVec3(const ufbx_vec3& v)
		{
			DataProtocol::Vec3 out{};
			out.x = (EZ::f32)v.x;
			out.y = (EZ::f32)v.y;
			out.z = (EZ::f32)v.z;
			return out;
		}

		static DataProtocol::Vec4 ToDPVec4(const ufbx_vec4& v)
		{
			DataProtocol::Vec4 out{};
			out.x = (EZ::f32)v.x;
			out.y = (EZ::f32)v.y;
			out.z = (EZ::f32)v.z;
			out.w = (EZ::f32)v.w;
			return out;
		}

		static DataProtocol::Quat ToDPQuat(const ufbx_quat& q)
		{
			DataProtocol::Quat out{};
			out.x = (EZ::f32)q.x;
			out.y = (EZ::f32)q.y;
			out.z = (EZ::f32)q.z;
			out.w = (EZ::f32)q.w;
			return out;
		}

		static DataProtocol::Mat4 ToDPMat4(const ufbx_matrix& m)
		{
			DataProtocol::Mat4 out{};

			out.m[0] = (EZ::f32)m.m00;
			out.m[1] = (EZ::f32)m.m10;
			out.m[2] = (EZ::f32)m.m20;
			out.m[3] = 0.0f;

			out.m[4] = (EZ::f32)m.m01;
			out.m[5] = (EZ::f32)m.m11;
			out.m[6] = (EZ::f32)m.m21;
			out.m[7] = 0.0f;

			out.m[8] = (EZ::f32)m.m02;
			out.m[9] = (EZ::f32)m.m12;
			out.m[10] = (EZ::f32)m.m22;
			out.m[11] = 0.0f;

			out.m[12] = (EZ::f32)m.m03;
			out.m[13] = (EZ::f32)m.m13;
			out.m[14] = (EZ::f32)m.m23;
			out.m[15] = 1.0f;

			return out;
		}

		static DataProtocol::Vec3 ScaleVec3(const DataProtocol::Vec3& v, EZ::f32 s)
		{
			DataProtocol::Vec3 out{};
			out.x = v.x * s;
			out.y = v.y * s;
			out.z = v.z * s;
			return out;
		}

		static void ScaleMatTranslation(DataProtocol::Mat4& m, EZ::f32 s)
		{
			m.m[12] *= s;
			m.m[13] *= s;
			m.m[14] *= s;
		}

		static std::string NormalizePath(const std::string& path)
		{
			if (path.empty()) {
				return {};
			}

			try {
				return fs::weakly_canonical(fs::path(path)).generic_string();
			}
			catch (...) {
				return fs::path(path).lexically_normal().generic_string();
			}
		}

		static std::string JoinPath(const fs::path& baseDir, const std::string& rel)
		{
			if (rel.empty()) {
				return {};
			}

			fs::path p(rel);
			if (p.is_absolute()) {
				return NormalizePath(p.generic_string());
			}

			return NormalizePath((baseDir / p).generic_string());
		}

		static bool HasSkin(const ufbx_mesh* mesh)
		{
			return mesh && mesh->skin_deformers.count > 0;
		}

		static ufbx_texture* ResolveFileTexture(ufbx_texture* tex)
		{
			if (!tex) return nullptr;

			if (tex->type == UFBX_TEXTURE_FILE) {
				return tex;
			}

			if (tex->file_textures.count > 0 && tex->file_textures[0]) {
				return tex->file_textures[0];
			}

			return tex;
		}

		static std::string ResolveTextureSourcePath(const fs::path& fbxDir, ufbx_texture* texture)
		{
			if (!texture) {
				return {};
			}

			texture = ResolveFileTexture(texture);
			if (!texture) {
				return {};
			}

			if (texture->filename.length > 0) {
				return JoinPath(fbxDir, ToString(texture->filename));
			}
			if (texture->relative_filename.length > 0) {
				return JoinPath(fbxDir, ToString(texture->relative_filename));
			}
			if (texture->absolute_filename.length > 0) {
				return NormalizePath(ToString(texture->absolute_filename));
			}

			std::string name = ToString(texture->name);
			if (name.empty()) {
				name = "EmbeddedTexture";
			}
			return NormalizePath((fbxDir / (name + ".embedded")).generic_string());
		}

		static DataProtocol::ImageFormat GuessImageFormat(EZ::u32 channels, bool srgb)
		{
			switch (channels)
			{
			case 1: return DataProtocol::ImageFormat::R8;
			case 2: return DataProtocol::ImageFormat::RG8;
			case 3: return DataProtocol::ImageFormat::RGB8;
			case 4: return srgb ? DataProtocol::ImageFormat::RGBA8_SRGB : DataProtocol::ImageFormat::RGBA8;
			default: return DataProtocol::ImageFormat::Unknown;
			}
		}

		static void FillImageInfoFromTexture(DataProtocol::ImageAsset& imageAsset, ufbx_texture* texture)
		{
			if (!texture) {
				return;
			}

			texture = ResolveFileTexture(texture);
			if (!texture) {
				return;
			}

			int w = 0;
			int h = 0;
			int comp = 0;

			if (texture->content.data && texture->content.size > 0)
			{
				if (stbi_info_from_memory(
					reinterpret_cast<const stbi_uc*>(texture->content.data),
					static_cast<int>(texture->content.size),
					&w, &h, &comp))
				{
					imageAsset.width = (EZ::u32)w;
					imageAsset.height = (EZ::u32)h;
					imageAsset.channels = (EZ::u32)comp;
					imageAsset.hasAlpha = comp == 4;
					imageAsset.format = GuessImageFormat(imageAsset.channels, imageAsset.srgb);
					return;
				}
			}

			if (!imageAsset.sourcePath.empty())
			{
				if (stbi_info(imageAsset.sourcePath.c_str(), &w, &h, &comp))
				{
					imageAsset.width = (EZ::u32)w;
					imageAsset.height = (EZ::u32)h;
					imageAsset.channels = (EZ::u32)comp;
					imageAsset.hasAlpha = comp == 4;
					imageAsset.format = GuessImageFormat(imageAsset.channels, imageAsset.srgb);
				}
			}
		}

		static DataProtocol::VertexBufferLayoutDesc MakeStaticVertexLayout()
		{
			using namespace DataProtocol;

			VertexBufferLayoutDesc layout{};
			layout.stride = sizeof(PackedStaticVertex);

			layout.attributes.push_back({
				VertexSemantic::Position, 0, 3, VertexScalarType::Float32, false,
				(EZ::u32)offsetof(PackedStaticVertex, px)
				});
			layout.attributes.push_back({
				VertexSemantic::Normal, 0, 3, VertexScalarType::Float32, false,
				(EZ::u32)offsetof(PackedStaticVertex, nx)
				});
			layout.attributes.push_back({
				VertexSemantic::UV, 0, 2, VertexScalarType::Float32, false,
				(EZ::u32)offsetof(PackedStaticVertex, u)
				});

			return layout;
		}

		static DataProtocol::VertexBufferLayoutDesc MakeSkinnedVertexLayout()
		{
			using namespace DataProtocol;

			VertexBufferLayoutDesc layout{};
			layout.stride = sizeof(PackedSkinnedVertex);

			layout.attributes.push_back({
				VertexSemantic::Position, 0, 3, VertexScalarType::Float32, false,
				(EZ::u32)offsetof(PackedSkinnedVertex, px)
				});
			layout.attributes.push_back({
				VertexSemantic::Normal, 0, 3, VertexScalarType::Float32, false,
				(EZ::u32)offsetof(PackedSkinnedVertex, nx)
				});
			layout.attributes.push_back({
				VertexSemantic::UV, 0, 2, VertexScalarType::Float32, false,
				(EZ::u32)offsetof(PackedSkinnedVertex, u)
				});
			layout.attributes.push_back({
				VertexSemantic::BoneIndices, 0, 4, VertexScalarType::Int32, false,
				(EZ::u32)offsetof(PackedSkinnedVertex, bi)
				});
			layout.attributes.push_back({
				VertexSemantic::BoneWeights, 0, 4, VertexScalarType::Float32, false,
				(EZ::u32)offsetof(PackedSkinnedVertex, bw)
				});

			return layout;
		}

		template<typename T>
		static void AppendBytes(std::vector<EZ::u8>& dst, const T& value)
		{
			const EZ::u8* begin = reinterpret_cast<const EZ::u8*>(&value);
			dst.insert(dst.end(), begin, begin + sizeof(T));
		}

		static void UpdateBounds(DataProtocol::AABBDesc& bounds, const DataProtocol::Vec3& p, bool& hasAny)
		{
			if (!hasAny)
			{
				bounds.min = p;
				bounds.max = p;
				hasAny = true;
				return;
			}

			bounds.min.x = std::min(bounds.min.x, p.x);
			bounds.min.y = std::min(bounds.min.y, p.y);
			bounds.min.z = std::min(bounds.min.z, p.z);

			bounds.max.x = std::max(bounds.max.x, p.x);
			bounds.max.y = std::max(bounds.max.y, p.y);
			bounds.max.z = std::max(bounds.max.z, p.z);
		}

		static DataProtocol::Vec4 GetMaterialBaseColor(ufbx_material* material)
		{
			DataProtocol::Vec4 out{ 1.0f, 1.0f, 1.0f, 1.0f };
			if (!material) {
				return out;
			}

			const auto& pbr = material->pbr.base_color;
			if (pbr.has_value)
			{
				if (pbr.value_components >= 4) {
					out = ToDPVec4(pbr.value_vec4);
				}
				else if (pbr.value_components == 3) {
					auto v = ToDPVec3(pbr.value_vec3);
					out.x = v.x;
					out.y = v.y;
					out.z = v.z;
					out.w = 1.0f;
				}
				return out;
			}

			const auto& fbx = material->fbx.diffuse_color;
			if (fbx.has_value)
			{
				if (fbx.value_components >= 4) {
					out = ToDPVec4(fbx.value_vec4);
				}
				else if (fbx.value_components == 3) {
					auto v = ToDPVec3(fbx.value_vec3);
					out.x = v.x;
					out.y = v.y;
					out.z = v.z;
					out.w = 1.0f;
				}
			}

			return out;
		}

		static ufbx_texture* GetMaterialBaseColorTexture(ufbx_material* material)
		{
			if (!material) return nullptr;

			if (material->pbr.base_color.texture && material->pbr.base_color.texture_enabled) {
				return material->pbr.base_color.texture;
			}
			if (material->fbx.diffuse_color.texture && material->fbx.diffuse_color.texture_enabled) {
				return material->fbx.diffuse_color.texture;
			}
			return nullptr;
		}

		static ufbx_texture* GetMaterialNormalTexture(ufbx_material* material)
		{
			if (!material) return nullptr;

			if (material->pbr.normal_map.texture && material->pbr.normal_map.texture_enabled) {
				return material->pbr.normal_map.texture;
			}
			if (material->fbx.normal_map.texture && material->fbx.normal_map.texture_enabled) {
				return material->fbx.normal_map.texture;
			}
			return nullptr;
		}

		static ufbx_texture* GetPreferredUVTexture(ufbx_material* material)
		{
			if (!material) {
				return nullptr;
			}

			if (ufbx_texture* baseTex = GetMaterialBaseColorTexture(material)) {
				return baseTex;
			}
			if (ufbx_texture* normalTex = GetMaterialNormalTexture(material)) {
				return normalTex;
			}

			return nullptr;
		}

		static const ufbx_vertex_vec2* FindUVLayerByTexturePreference(
			const ufbx_mesh* mesh,
			ufbx_texture* texture,
			bool preferTextureSpecifiedUVSet)
		{
			if (!mesh) {
				return nullptr;
			}

			texture = ResolveFileTexture(texture);

			if (preferTextureSpecifiedUVSet && texture && texture->uv_set.length > 0)
			{
				const std::string targetUVSet = ToString(texture->uv_set);

				for (size_t i = 0; i < mesh->uv_sets.count; ++i)
				{
					const ufbx_uv_set& uvSet = mesh->uv_sets[i];
					if (!uvSet.vertex_uv.exists) {
						continue;
					}
					if (ToString(uvSet.name) == targetUVSet) {
						return &uvSet.vertex_uv;
					}
				}
			}

			if (mesh->vertex_uv.exists) {
				return &mesh->vertex_uv;
			}

			for (size_t i = 0; i < mesh->uv_sets.count; ++i)
			{
				const ufbx_uv_set& uvSet = mesh->uv_sets[i];
				if (uvSet.vertex_uv.exists) {
					return &uvSet.vertex_uv;
				}
			}

			return nullptr;
		}

		static DataProtocol::Vec2 ApplyTextureUVTransform(
			ufbx_texture* texture,
			const DataProtocol::Vec2& uv,
			bool applyTextureUVTransform)
		{
			DataProtocol::Vec2 out = uv;

			if (!applyTextureUVTransform) {
				return out;
			}

			texture = ResolveFileTexture(texture);
			if (!texture) {
				return out;
			}

			if (texture->has_uv_transform)
			{
				ufbx_vec3 p{};
				p.x = uv.x;
				p.y = uv.y;
				p.z = 0.0;

				const ufbx_vec3 transformed = ufbx_transform_position(&texture->uv_to_texture, p);
				out.x = (EZ::f32)transformed.x;
				out.y = (EZ::f32)transformed.y;
			}

			return out;
		}

		static DataProtocol::Vec2 SampleMeshUV(
			const ufbx_mesh* mesh,
			EZ::u32 cornerIndex,
			ufbx_material* material,
			const FBXImportOptions& options)
		{
			DataProtocol::Vec2 uv{ 0.0f, 0.0f };

			if (!mesh) {
				return uv;
			}

			ufbx_texture* preferredTexture = GetPreferredUVTexture(material);

			const ufbx_vertex_vec2* uvLayer = FindUVLayerByTexturePreference(
				mesh,
				preferredTexture,
				options.preferTextureSpecifiedUVSet);

			if (uvLayer) {
				uv = ToDPVec2((*uvLayer)[cornerIndex]);
			}

			uv = ApplyTextureUVTransform(
				preferredTexture,
				uv,
				options.applyTextureUVTransform);

			if (options.flipVForOpenGL) {
				uv.y = 1.0f - uv.y;
			}

			return uv;
		}

		static bool MaterialLooksTransparent(ufbx_material* material)
		{
			if (!material) return false;

			const auto& opacity = material->pbr.opacity;
			if (opacity.texture && opacity.texture_enabled) {
				return true;
			}
			if (opacity.has_value && opacity.value_components >= 1 && opacity.value_real < 0.999f) {
				return true;
			}

			const auto& transFactor = material->fbx.transparency_factor;
			if (transFactor.texture && transFactor.texture_enabled) {
				return true;
			}
			if (transFactor.has_value && transFactor.value_components >= 1 && transFactor.value_real > 0.001f) {
				return true;
			}

			return false;
		}

		static std::vector<const ufbx_node*> CollectMeshNodes(const ufbx_scene* scene)
		{
			std::vector<const ufbx_node*> out;
			if (!scene) return out;

			out.reserve(scene->nodes.count);
			for (size_t i = 0; i < scene->nodes.count; ++i)
			{
				const ufbx_node* node = scene->nodes[i];
				if (node && node->mesh) {
					out.push_back(node);
				}
			}
			return out;
		}

		static std::unordered_set<const ufbx_node*> CollectUsedSkeletonNodes(const std::vector<const ufbx_node*>& meshNodes)
		{
			std::unordered_set<const ufbx_node*> usedNodes;

			for (const ufbx_node* node : meshNodes)
			{
				if (!node || !node->mesh || !HasSkin(node->mesh)) {
					continue;
				}

				const ufbx_mesh* mesh = node->mesh;
				for (size_t sd = 0; sd < mesh->skin_deformers.count; ++sd)
				{
					const ufbx_skin_deformer* skin = mesh->skin_deformers[sd];
					if (!skin) continue;

					for (size_t ci = 0; ci < skin->clusters.count; ++ci)
					{
						const ufbx_skin_cluster* cluster = skin->clusters[ci];
						if (!cluster || !cluster->bone_node) continue;

						const ufbx_node* p = cluster->bone_node;
						while (p) {
							usedNodes.insert(p);
							p = p->parent;
						}
					}
				}
			}

			return usedNodes;
		}

		static std::unique_ptr<DataProtocol::SkeletonAsset> BuildSkeletonAsset(
			const std::string& sourcePath,
			const ufbx_scene* scene,
			const std::unordered_set<const ufbx_node*>& usedNodes,
			EZ::f32 globalScale)
		{
			auto skeleton = std::make_unique<DataProtocol::SkeletonAsset>();
			skeleton->sourcePath = sourcePath;
			skeleton->isImported = true;

			if (!scene || usedNodes.empty()) {
				return skeleton;
			}

			std::unordered_map<std::string, EZ::i32> boneIndexByName;

			for (size_t i = 0; i < scene->nodes.count; ++i)
			{
				const ufbx_node* node = scene->nodes[i];
				if (!node) continue;
				if (usedNodes.find(node) == usedNodes.end()) continue;

				DataProtocol::BoneDesc bone{};
				bone.name = ToString(node->name);
				bone.parentIndex = -1;

				bone.localBindPosition = ScaleVec3(ToDPVec3(node->local_transform.translation), globalScale);
				bone.localBindRotation = ToDPQuat(node->local_transform.rotation);
				bone.localBindScale = ToDPVec3(node->local_transform.scale);

				boneIndexByName[bone.name] = (EZ::i32)skeleton->bones.size();
				skeleton->bones.push_back(bone);
			}

			for (size_t i = 0; i < scene->nodes.count; ++i)
			{
				const ufbx_node* node = scene->nodes[i];
				if (!node) continue;
				if (usedNodes.find(node) == usedNodes.end()) continue;

				auto selfIt = boneIndexByName.find(ToString(node->name));
				if (selfIt == boneIndexByName.end()) continue;

				EZ::i32 selfIndex = selfIt->second;

				const ufbx_node* p = node->parent;
				while (p)
				{
					auto parentIt = boneIndexByName.find(ToString(p->name));
					if (parentIt != boneIndexByName.end())
					{
						skeleton->bones[selfIndex].parentIndex = parentIt->second;
						break;
					}
					p = p->parent;
				}
			}

			skeleton->rootBoneIndex = -1;
			for (EZ::u32 i = 0; i < (EZ::u32)skeleton->bones.size(); ++i)
			{
				if (skeleton->bones[i].parentIndex < 0) {
					skeleton->rootBoneIndex = (EZ::i32)i;
					break;
				}
			}

			return skeleton;
		}

		static void BuildPerVertexWeights(
			const ufbx_mesh* mesh,
			const std::unordered_map<std::string, EZ::i32>& boneIndexByName,
			EZ::u32 maxBonesPerVertex,
			std::vector<DataProtocol::IVec4>& outBoneIndices,
			std::vector<DataProtocol::Vec4>& outBoneWeights,
			std::vector<TempBoneBinding>& skinBindings,
			EZ::f32 globalScale)
		{
			outBoneIndices.assign(mesh->num_vertices, DataProtocol::IVec4{});
			outBoneWeights.assign(mesh->num_vertices, DataProtocol::Vec4{});

			std::vector<std::vector<std::pair<EZ::i32, EZ::f32>>> perVertex(mesh->num_vertices);

			for (size_t sd = 0; sd < mesh->skin_deformers.count; ++sd)
			{
				const ufbx_skin_deformer* skin = mesh->skin_deformers[sd];
				if (!skin) continue;

				for (size_t ci = 0; ci < skin->clusters.count; ++ci)
				{
					const ufbx_skin_cluster* cluster = skin->clusters[ci];
					if (!cluster || !cluster->bone_node) continue;

					const std::string boneName = ToString(cluster->bone_node->name);
					auto boneIt = boneIndexByName.find(boneName);
					if (boneIt == boneIndexByName.end()) {
						continue;
					}

					EZ::i32 boneIndex = boneIt->second;

					if (boneIndex >= 0 && boneIndex < (EZ::i32)skinBindings.size())
					{
						if (!skinBindings[boneIndex].valid)
						{
							skinBindings[boneIndex].valid = true;
							skinBindings[boneIndex].boneName = boneName;
							skinBindings[boneIndex].skeletonBoneIndex = boneIndex;
							skinBindings[boneIndex].inverseBindMatrix = ToDPMat4(cluster->geometry_to_bone);
							ScaleMatTranslation(skinBindings[boneIndex].inverseBindMatrix, globalScale);
						}
					}

					for (size_t w = 0; w < cluster->num_weights; ++w)
					{
						EZ::u32 logicalVertex = cluster->vertices[w];
						EZ::f32 weight = (EZ::f32)cluster->weights[w];

						if (logicalVertex >= mesh->num_vertices) continue;
						if (weight <= 0.0f) continue;

						perVertex[logicalVertex].emplace_back(boneIndex, weight);
					}
				}
			}

			for (size_t v = 0; v < perVertex.size(); ++v)
			{
				auto& weights = perVertex[v];
				if (weights.empty()) {
					continue;
				}

				std::sort(weights.begin(), weights.end(),
					[](const auto& a, const auto& b) {
						return a.second > b.second;
					});

				EZ::u32 count = std::min<EZ::u32>(maxBonesPerVertex, (EZ::u32)weights.size());
				EZ::f32 sum = 0.0f;
				for (EZ::u32 i = 0; i < count; ++i) {
					sum += weights[i].second;
				}
				if (sum <= 0.0f) continue;

				for (EZ::u32 i = 0; i < count && i < 4; ++i)
				{
					const EZ::i32 boneIndex = weights[i].first;
					const EZ::f32 normalizedWeight = weights[i].second / sum;

					if (i == 0) { outBoneIndices[v].x = boneIndex; outBoneWeights[v].x = normalizedWeight; }
					if (i == 1) { outBoneIndices[v].y = boneIndex; outBoneWeights[v].y = normalizedWeight; }
					if (i == 2) { outBoneIndices[v].z = boneIndex; outBoneWeights[v].z = normalizedWeight; }
					if (i == 3) { outBoneIndices[v].w = boneIndex; outBoneWeights[v].w = normalizedWeight; }
				}
			}
		}

		static std::unique_ptr<DataProtocol::MeshAsset> BuildMergedMeshAsset(
			const std::string& sourcePath,
			const std::vector<const ufbx_node*>& meshNodes,
			const std::unordered_map<std::string, EZ::i32>& boneIndexByName,
			std::vector<TempBoneBinding>& skinBindings,
			std::vector<ufbx_material*>& mergedMaterials,
			const FBXImportOptions& options)
		{
			auto meshAsset = std::make_unique<DataProtocol::MeshAsset>();
			meshAsset->sourcePath = sourcePath;
			meshAsset->isImported = true;

			bool anySkinning = false;
			for (const ufbx_node* node : meshNodes)
			{
				if (node && node->mesh && HasSkin(node->mesh)) {
					anySkinning = true;
					break;
				}
			}

			meshAsset->hasSkinning = anySkinning;
			meshAsset->vertexLayout = anySkinning ? MakeSkinnedVertexLayout() : MakeStaticVertexLayout();
			meshAsset->indexType = DataProtocol::IndexScalarType::UInt32;

			std::unordered_map<ufbx_material*, EZ::u32> materialSlotMap;
			bool hasAnyBounds = false;

			EZ::u32 vertexCount = 0;
			EZ::u32 indexCount = 0;

			for (const ufbx_node* node : meshNodes)
			{
				if (!node || !node->mesh) {
					continue;
				}

				const ufbx_mesh* mesh = node->mesh;

				std::vector<DataProtocol::IVec4> logicalBoneIndices;
				std::vector<DataProtocol::Vec4> logicalBoneWeights;

				if (anySkinning) {
					BuildPerVertexWeights(
						mesh,
						boneIndexByName,
						options.maxBonesPerVertex,
						logicalBoneIndices,
						logicalBoneWeights,
						skinBindings,
						options.globalScale);
				}

				for (size_t pi = 0; pi < mesh->material_parts.count; ++pi)
				{
					const ufbx_mesh_part& part = mesh->material_parts[pi];

					ufbx_material* material = nullptr;
					if (part.index < node->materials.count) {
						material = node->materials[part.index];
					}
					else if (part.index < mesh->materials.count) {
						material = mesh->materials[part.index];
					}

					EZ::u32 materialSlot = 0;
					auto matIt = materialSlotMap.find(material);
					if (matIt == materialSlotMap.end())
					{
						materialSlot = (EZ::u32)mergedMaterials.size();
						mergedMaterials.push_back(material);
						materialSlotMap.emplace(material, materialSlot);
					}
					else {
						materialSlot = matIt->second;
					}

					DataProtocol::SubMeshDesc sub{};
					sub.indexOffset = indexCount;
					sub.indexCount = 0;
					sub.materialSlot = materialSlot;

					for (size_t fi = 0; fi < part.face_indices.count; ++fi)
					{
						EZ::u32 faceIndex = part.face_indices[fi];
						const ufbx_face face = mesh->faces[faceIndex];
						if (face.num_indices < 3) {
							continue;
						}

						std::vector<EZ::u32> triIndices((face.num_indices - 2) * 3);
						EZ::u32 numTriangles = ufbx_triangulate_face(
							triIndices.data(),
							triIndices.size(),
							mesh,
							face);

						for (EZ::u32 i = 0; i < numTriangles * 3; ++i)
						{
							EZ::u32 cornerIndex = triIndices[i];
							EZ::u32 logicalVertex = mesh->vertex_indices[cornerIndex];

							DataProtocol::Vec3 pos = ScaleVec3(ToDPVec3(mesh->vertex_position[cornerIndex]), options.globalScale);
							DataProtocol::Vec3 nrm = mesh->vertex_normal.exists
								? ToDPVec3(mesh->vertex_normal[cornerIndex])
								: DataProtocol::Vec3{ 0.0f, 1.0f, 0.0f };

							DataProtocol::Vec2 uv = SampleMeshUV(
								mesh,
								cornerIndex,
								material,
								options);

							if (anySkinning)
							{
								PackedSkinnedVertex v{};
								v.px = pos.x; v.py = pos.y; v.pz = pos.z;
								v.nx = nrm.x; v.ny = nrm.y; v.nz = nrm.z;
								v.u = uv.x; v.v = uv.y;

								if (logicalVertex < logicalBoneIndices.size())
								{
									const auto& bi = logicalBoneIndices[logicalVertex];
									const auto& bw = logicalBoneWeights[logicalVertex];
									v.bi[0] = bi.x; v.bi[1] = bi.y; v.bi[2] = bi.z; v.bi[3] = bi.w;
									v.bw[0] = bw.x; v.bw[1] = bw.y; v.bw[2] = bw.z; v.bw[3] = bw.w;
								}
								else
								{
									v.bi[0] = 0; v.bi[1] = 0; v.bi[2] = 0; v.bi[3] = 0;
									v.bw[0] = 0.0f; v.bw[1] = 0.0f; v.bw[2] = 0.0f; v.bw[3] = 0.0f;
								}

								AppendBytes(meshAsset->vertexRawData, v);
							}
							else
							{
								PackedStaticVertex v{};
								v.px = pos.x; v.py = pos.y; v.pz = pos.z;
								v.nx = nrm.x; v.ny = nrm.y; v.nz = nrm.z;
								v.u = uv.x; v.v = uv.y;

								AppendBytes(meshAsset->vertexRawData, v);
							}

							AppendBytes(meshAsset->indexRawData, vertexCount);

							UpdateBounds(meshAsset->bounds, pos, hasAnyBounds);

							++vertexCount;
							++indexCount;
							++sub.indexCount;
						}
					}

					if (sub.indexCount > 0) {
						meshAsset->subMeshes.push_back(sub);
					}
				}
			}

			meshAsset->vertexCount = vertexCount;
			meshAsset->indexCount = indexCount;

			return meshAsset;
		}

		static std::vector<std::unique_ptr<DataProtocol::ImageAsset>> BuildImageAssets(
			const std::string& fbxPath,
			const std::vector<ufbx_material*>& mergedMaterials,
			std::unordered_map<std::string, std::string>& imageSourcePathMap)
		{
			std::vector<std::unique_ptr<DataProtocol::ImageAsset>> result;
			imageSourcePathMap.clear();

			fs::path fbxDir = fs::path(fbxPath).parent_path();

			auto addTextureImage = [&](ufbx_texture* texture, bool srgb)
				{
					if (!texture) return;

					texture = ResolveFileTexture(texture);
					if (!texture) return;

					std::string sourcePath = ResolveTextureSourcePath(fbxDir, texture);
					if (sourcePath.empty()) return;

					auto found = imageSourcePathMap.find(sourcePath);
					if (found != imageSourcePathMap.end()) {
						return;
					}

					auto image = std::make_unique<DataProtocol::ImageAsset>();
					image->sourcePath = sourcePath;
					image->isImported = true;
					image->dimension = DataProtocol::ImageDimension::Texture2D;
					image->srgb = srgb;

					FillImageInfoFromTexture(*image, texture);

					imageSourcePathMap[sourcePath] = sourcePath;
					result.push_back(std::move(image));
				};

			for (ufbx_material* material : mergedMaterials)
			{
				if (!material) continue;

				addTextureImage(GetMaterialBaseColorTexture(material), true);
				addTextureImage(GetMaterialNormalTexture(material), false);
			}

			return result;
		}

		static std::vector<std::unique_ptr<DataProtocol::MaterialAsset>> BuildMaterialAssets(
			const std::string& fbxPath,
			const std::vector<ufbx_material*>& mergedMaterials,
			const std::unordered_map<std::string, std::string>& imageSourcePathMap,
			bool preferCharacterShading)
		{
			std::vector<std::unique_ptr<DataProtocol::MaterialAsset>> result;
			result.reserve(mergedMaterials.size());

			fs::path fbxDir = fs::path(fbxPath).parent_path();

			for (EZ::u32 i = 0; i < (EZ::u32)mergedMaterials.size(); ++i)
			{
				ufbx_material* material = mergedMaterials[i];

				auto mat = std::make_unique<DataProtocol::MaterialAsset>();
				mat->sourcePath = NormalizePath(fbxPath) + "#Material/" + std::to_string(i);
				mat->isImported = true;

				mat->shadingModel = preferCharacterShading
					? DataProtocol::MaterialShadingModel::Character
					: DataProtocol::MaterialShadingModel::Lit;

				mat->baseColor = GetMaterialBaseColor(material);

				if (material && material->features.double_sided.enabled) {
					mat->doubleSided = true;
					mat->cullMode = DataProtocol::MaterialCullMode::None;
				}
				else {
					mat->doubleSided = false;
					mat->cullMode = DataProtocol::MaterialCullMode::Back;
				}

				mat->blendMode = MaterialLooksTransparent(material)
					? DataProtocol::MaterialBlendMode::AlphaBlend
					: DataProtocol::MaterialBlendMode::Opaque;

				mat->depthTest = true;
				mat->depthWrite = (mat->blendMode == DataProtocol::MaterialBlendMode::Opaque);

				if (material)
				{
					if (ufbx_texture* baseTex = GetMaterialBaseColorTexture(material))
					{
						std::string imagePath = ResolveTextureSourcePath(fbxDir, baseTex);
						if (!imagePath.empty() && imageSourcePathMap.find(imagePath) != imageSourcePathMap.end())
						{
							DataProtocol::MaterialTextureSlot slot{};
							slot.slotName = "BaseColor";
							slot.imageAssetPath = imagePath;
							slot.enabled = true;
							slot.srgb = true;
							mat->textureSlots.push_back(slot);
						}
					}

					if (ufbx_texture* normalTex = GetMaterialNormalTexture(material))
					{
						std::string imagePath = ResolveTextureSourcePath(fbxDir, normalTex);
						if (!imagePath.empty() && imageSourcePathMap.find(imagePath) != imageSourcePathMap.end())
						{
							DataProtocol::MaterialTextureSlot slot{};
							slot.slotName = "Normal";
							slot.imageAssetPath = imagePath;
							slot.enabled = true;
							slot.srgb = false;
							mat->textureSlots.push_back(slot);
						}
					}
				}

				result.push_back(std::move(mat));
			}

			if (result.empty())
			{
				auto mat = std::make_unique<DataProtocol::MaterialAsset>();
				mat->sourcePath = NormalizePath(fbxPath) + "#Material/Default";
				mat->isImported = true;
				mat->shadingModel = preferCharacterShading
					? DataProtocol::MaterialShadingModel::Character
					: DataProtocol::MaterialShadingModel::Lit;
				mat->baseColor = DataProtocol::Vec4{ 1.0f, 1.0f, 1.0f, 1.0f };
				result.push_back(std::move(mat));
			}

			return result;
		}

		static std::vector<std::unique_ptr<DataProtocol::AnimationClipAsset>> BuildAnimationAssets(
			const std::string& fbxPath,
			const ufbx_scene* scene,
			const DataProtocol::SkeletonAsset* skeletonAsset,
			EZ::f32 globalScale)
		{
			std::vector<std::unique_ptr<DataProtocol::AnimationClipAsset>> result;
			if (!scene || !skeletonAsset || scene->anim_stacks.count == 0) {
				return result;
			}

			std::unordered_map<std::string, EZ::i32> boneIndexByName;
			for (EZ::u32 i = 0; i < (EZ::u32)skeletonAsset->bones.size(); ++i) {
				boneIndexByName[skeletonAsset->bones[i].name] = (EZ::i32)i;
			}

			const EZ::f32 sampleRate = 30.0f;

			for (size_t si = 0; si < scene->anim_stacks.count; ++si)
			{
				const ufbx_anim_stack* stack = scene->anim_stacks[si];
				if (!stack || !stack->anim) {
					continue;
				}

				auto clip = std::make_unique<DataProtocol::AnimationClipAsset>();
				clip->sourcePath = NormalizePath(fbxPath) + "#AnimStack/" + ToString(stack->name);
				clip->isImported = true;
				clip->clipName = ToString(stack->name);
				if (clip->clipName.empty()) {
					clip->clipName = fs::path(fbxPath).stem().string();
				}

				const double timeBegin = stack->time_begin;
				const double timeEnd = stack->time_end;
				const double duration = std::max(0.0, timeEnd - timeBegin);

				clip->duration = (EZ::f32)duration;
				clip->ticksPerSecond = sampleRate;

				clip->tracks.resize(skeletonAsset->bones.size());
				for (EZ::u32 bi = 0; bi < (EZ::u32)skeletonAsset->bones.size(); ++bi)
				{
					clip->tracks[bi].boneName = skeletonAsset->bones[bi].name;
					clip->tracks[bi].skeletonBoneIndex = (EZ::i32)bi;
				}

				const EZ::u32 frameCount = std::max<EZ::u32>(1, (EZ::u32)std::ceil(duration * sampleRate) + 1);

				for (EZ::u32 frame = 0; frame < frameCount; ++frame)
				{
					const double localTime = duration <= 0.0
						? 0.0
						: std::min(duration, (double)frame / (double)sampleRate);
					const double evalTime = timeBegin + localTime;

					ufbx_error error{};
					ScenePtr evalScene(ufbx_evaluate_scene(scene, stack->anim, evalTime, nullptr, &error));
					if (!evalScene) {
						continue;
					}

					std::unordered_map<std::string, const ufbx_node*> evalNodeByName;
					evalNodeByName.reserve(evalScene->nodes.count);
					for (size_t ni = 0; ni < evalScene->nodes.count; ++ni)
					{
						const ufbx_node* node = evalScene->nodes[ni];
						if (node) {
							evalNodeByName.emplace(ToString(node->name), node);
						}
					}

					for (EZ::u32 bi = 0; bi < (EZ::u32)skeletonAsset->bones.size(); ++bi)
					{
						const std::string& boneName = skeletonAsset->bones[bi].name;
						auto nodeIt = evalNodeByName.find(boneName);
						if (nodeIt == evalNodeByName.end()) {
							continue;
						}

						const ufbx_node* node = nodeIt->second;
						if (!node) {
							continue;
						}

						DataProtocol::AnimationVecKey posKey{};
						posKey.time = (EZ::f32)localTime;
						posKey.value = ScaleVec3(ToDPVec3(node->local_transform.translation), globalScale);

						DataProtocol::AnimationQuatKey rotKey{};
						rotKey.time = (EZ::f32)localTime;
						rotKey.value = ToDPQuat(node->local_transform.rotation);

						DataProtocol::AnimationVecKey scaleKey{};
						scaleKey.time = (EZ::f32)localTime;
						scaleKey.value = ToDPVec3(node->local_transform.scale);

						auto& track = clip->tracks[bi];
						track.positionKeys.push_back(posKey);
						track.rotationKeys.push_back(rotKey);
						track.scaleKeys.push_back(scaleKey);
					}
				}

				result.push_back(std::move(clip));
			}

			return result;
		}
	}

	FBXImportResult FBXImporter::Import(const std::string& fbxPath, const FBXImportOptions& options)
	{
		FBXImportResult result;

		ufbx_load_opts loadOpts{};
		loadOpts.generate_missing_normals = options.generateMissingNormals;
		loadOpts.clean_skin_weights = options.cleanSkinWeights;

		ufbx_error error{};
		ScenePtr scene(ufbx_load_file(fbxPath.c_str(), &loadOpts, &error));
		if (!scene)
		{
			char buffer[512];
			ufbx_format_error(buffer, sizeof(buffer), &error);
			throw std::runtime_error(std::string("FBXImporter load failed: ") + buffer);
		}

		const std::string normalizedFbxPath = NormalizePath(fbxPath);
		const auto meshNodes = CollectMeshNodes(scene.get());

		bool anySkinning = false;
		for (const ufbx_node* node : meshNodes)
		{
			if (node && node->mesh && HasSkin(node->mesh)) {
				anySkinning = true;
				break;
			}
		}

		std::unordered_set<const ufbx_node*> usedSkeletonNodes;
		std::unordered_map<std::string, EZ::i32> boneIndexByName;

		if (options.importSkeleton && anySkinning)
		{
			usedSkeletonNodes = CollectUsedSkeletonNodes(meshNodes);

			result.skeletonAsset = BuildSkeletonAsset(
				normalizedFbxPath + "#Skeleton",
				scene.get(),
				usedSkeletonNodes,
				options.globalScale);

			if (result.skeletonAsset)
			{
				for (EZ::u32 i = 0; i < (EZ::u32)result.skeletonAsset->bones.size(); ++i) {
					boneIndexByName[result.skeletonAsset->bones[i].name] = (EZ::i32)i;
				}
			}
		}

		std::vector<TempBoneBinding> skinBindings;
		if (result.skeletonAsset) {
			skinBindings.resize(result.skeletonAsset->bones.size());
		}

		std::vector<ufbx_material*> mergedMaterials;

		if (options.importMesh)
		{
			result.meshAsset = BuildMergedMeshAsset(
				normalizedFbxPath + "#Mesh",
				meshNodes,
				boneIndexByName,
				skinBindings,
				mergedMaterials,
				options);
		}

		if (options.importSkin && result.meshAsset && result.meshAsset->hasSkinning && result.skeletonAsset)
		{
			auto skin = std::make_unique<DataProtocol::SkinAsset>();
			skin->sourcePath = normalizedFbxPath + "#Skin";
			skin->isImported = true;
			skin->meshAssetPath = result.meshAsset->sourcePath;
			skin->skeletonAssetPath = result.skeletonAsset->sourcePath;
			skin->maxBonesPerVertex = options.maxBonesPerVertex;

			skin->boneBindings.reserve(skinBindings.size());
			for (EZ::u32 i = 0; i < (EZ::u32)skinBindings.size(); ++i)
			{
				DataProtocol::SkinBoneBindingDesc binding{};
				if (skinBindings[i].valid)
				{
					binding.boneName = skinBindings[i].boneName;
					binding.skeletonBoneIndex = skinBindings[i].skeletonBoneIndex;
					binding.inverseBindMatrix = skinBindings[i].inverseBindMatrix;
				}
				else
				{
					if (i < result.skeletonAsset->bones.size()) {
						binding.boneName = result.skeletonAsset->bones[i].name;
						binding.skeletonBoneIndex = (EZ::i32)i;
						binding.inverseBindMatrix = DataProtocol::Mat4{};
					}
				}
				skin->boneBindings.push_back(binding);
			}

			result.skinAsset = std::move(skin);
		}

		std::unordered_map<std::string, std::string> imageSourcePathMap;
		if (options.importImages)
		{
			result.imageAssets = BuildImageAssets(
				normalizedFbxPath,
				mergedMaterials,
				imageSourcePathMap);
		}

		if (options.importMaterials)
		{
			result.materialAssets = BuildMaterialAssets(
				normalizedFbxPath,
				mergedMaterials,
				imageSourcePathMap,
				anySkinning);
		}

		if (options.importAnimations && result.skeletonAsset)
		{
			result.animationAssets = BuildAnimationAssets(
				normalizedFbxPath,
				scene.get(),
				result.skeletonAsset.get(),
				options.globalScale);
		}

		return result;
	}
}