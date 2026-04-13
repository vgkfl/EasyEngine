#include "RenderSystem.h"

#include <SDL3/SDL.h>
#include <GL/glew.h>

#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

#include "stb_image/stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "BaseProtocol/Animation/SkinningPaletteComponent.h"
#include "BaseProtocol/Camera/CameraComponent.h"
#include "BaseProtocol/Light/LightComponent.h"
#include "BaseProtocol/Mesh/MeshRenderComponent.h"
#include "BaseProtocol/Mesh/SkinnedMeshRendererComponent.h"
#include "BaseProtocol/Transform/Transform.h"
#include "ControlProtocol/EntityManager/EntityManager.h"
#include "ControlProtocol/RenderDeviceController/RenderDeviceController.h"
#include "ControlProtocol/RenderPipelineController/RenderImageBuffer.h"
#include "ControlProtocol/WindowController/WindowController.h"
#include "core/Context/RunTimeContext/ProjectContext.h"
#include "core/Context/RunTimeContext/WorldContext.h"

namespace
{
	struct ParsedShaderSource
	{
		std::string vertex;
		std::string fragment;
	};

	glm::mat4 ToGLM(const DataProtocol::Mat4& m)
	{
		glm::mat4 result(1.0f);
		for (EZ::u32 i = 0; i < 16; ++i)
		{
			result[i / 4][i % 4] = m.m[i];
		}
		return result;
	}

	glm::vec3 ToGLM(const DataProtocol::Vec3& v)
	{
		return glm::vec3(v.x, v.y, v.z);
	}

	glm::vec4 ToGLM(const DataProtocol::Vec4& v)
	{
		return glm::vec4(v.x, v.y, v.z, v.w);
	}

	glm::vec3 ExtractTranslation(const DataProtocol::Mat4& m)
	{
		return glm::vec3(m.m[12], m.m[13], m.m[14]);
	}

	glm::vec3 ExtractForward(const DataProtocol::Mat4& m)
	{
		glm::vec3 forward(-m.m[8], -m.m[9], -m.m[10]);
		const float len = glm::length(forward);
		if (len <= 0.0001f)
		{
			return glm::vec3(0.0f, 0.0f, -1.0f);
		}
		return forward / len;
	}

	GLenum ToGLScalarType(DataProtocol::VertexScalarType type)
	{
		switch (type)
		{
		case DataProtocol::VertexScalarType::Float32: return GL_FLOAT;
		case DataProtocol::VertexScalarType::Int32:   return GL_INT;
		case DataProtocol::VertexScalarType::UInt32:  return GL_UNSIGNED_INT;
		case DataProtocol::VertexScalarType::UInt16:  return GL_UNSIGNED_SHORT;
		case DataProtocol::VertexScalarType::UInt8:   return GL_UNSIGNED_BYTE;
		default:                                      return 0;
		}
	}

	EZ::u32 CompileShader(GLenum type, const char* source)
	{
		GLuint shader = glCreateShader(type);
		glShaderSource(shader, 1, &source, nullptr);
		glCompileShader(shader);

		GLint success = GL_FALSE;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (success == GL_TRUE)
		{
			return (EZ::u32)shader;
		}

		char log[2048]{};
		glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
		std::printf("[RenderSystem] Shader compile failed:\n%s\n", log);

		glDeleteShader(shader);
		return 0;
	}

	EZ::u32 LinkProgram(EZ::u32 vs, EZ::u32 fs)
	{
		GLuint program = glCreateProgram();
		glAttachShader(program, vs);
		glAttachShader(program, fs);
		glLinkProgram(program);

		GLint success = GL_FALSE;
		glGetProgramiv(program, GL_LINK_STATUS, &success);
		if (success == GL_TRUE)
		{
			return (EZ::u32)program;
		}

		char log[2048]{};
		glGetProgramInfoLog(program, sizeof(log), nullptr, log);
		std::printf("[RenderSystem] Program link failed:\n%s\n", log);

		glDeleteProgram(program);
		return 0;
	}

	bool LoadTextFile(const std::string& path, std::string& outText)
	{
		std::ifstream ifs(path, std::ios::binary);
		if (!ifs.is_open())
		{
			return false;
		}

		std::ostringstream oss;
		oss << ifs.rdbuf();
		outText = oss.str();
		return true;
	}

	bool ParseShaderFile(const std::string& path, ParsedShaderSource& outSource)
	{
		std::string fileText;
		if (!LoadTextFile(path, fileText))
		{
			std::printf("[RenderSystem] Failed to open shader file: %s\n", path.c_str());
			return false;
		}

		std::stringstream ss(fileText);
		std::string line;
		std::stringstream vertexSS;
		std::stringstream fragmentSS;

		enum class Mode
		{
			None,
			Vertex,
			Fragment
		};

		Mode mode = Mode::None;

		while (std::getline(ss, line))
		{
			if (line.find("#shader vertex") != std::string::npos)
			{
				mode = Mode::Vertex;
				continue;
			}
			if (line.find("#shader fragment") != std::string::npos)
			{
				mode = Mode::Fragment;
				continue;
			}

			if (mode == Mode::Vertex)
			{
				vertexSS << line << "\n";
			}
			else if (mode == Mode::Fragment)
			{
				fragmentSS << line << "\n";
			}
		}

		outSource.vertex = vertexSS.str();
		outSource.fragment = fragmentSS.str();

		if (outSource.vertex.empty() || outSource.fragment.empty())
		{
			std::printf("[RenderSystem] Invalid shader file (missing vertex/fragment blocks): %s\n", path.c_str());
			return false;
		}

		return true;
	}

	const char* FindTextureSlot(const DataProtocol::MaterialAsset* material, const char* slotName)
	{
		if (!material)
		{
			return nullptr;
		}

		for (const auto& slot : material->textureSlots)
		{
			if (slot.enabled && slot.slotName == slotName)
			{
				return slot.imageAssetPath.c_str();
			}
		}

		return nullptr;
	}
}

int RenderSystem::Initialize(EZ::ProjectContext& project, EZ::WorldContext& world)
{
	(void)project;
	(void)world;

	glGenBuffers(1, reinterpret_cast<GLuint*>(&m_InstanceSSBO));
	glGenBuffers(1, reinterpret_cast<GLuint*>(&m_BoneSSBO));

	m_FrameData.Clear();
	m_FrameDataValid = false;

	return 0;
}

void RenderSystem::Shutdown(EZ::ProjectContext& project, EZ::WorldContext& world)
{
	(void)project;
	(void)world;

	DestroyAllMeshes();
	DestroyAllShaders();
	DestroyAllTextures();

	if (m_InstanceSSBO != 0)
	{
		glDeleteBuffers(1, reinterpret_cast<const GLuint*>(&m_InstanceSSBO));
		m_InstanceSSBO = 0;
	}
	if (m_BoneSSBO != 0)
	{
		glDeleteBuffers(1, reinterpret_cast<const GLuint*>(&m_BoneSSBO));
		m_BoneSSBO = 0;
	}

	m_FrameData.Clear();
	m_FrameDataValid = false;
}

void RenderSystem::BeginFrame(EZ::ProjectContext& project, EZ::WorldContext& world, float deltaTime)
{
	(void)project;
	(void)world;
	(void)deltaTime;

	m_FrameData.Clear();
	m_FrameDataValid = false;
	m_FrameStats.Reset();
}

void RenderSystem::DestroyAllMeshes()
{
	for (auto& [mesh, gpu] : m_MeshCache)
	{
		(void)mesh;
		if (gpu.ebo != 0) glDeleteBuffers(1, reinterpret_cast<const GLuint*>(&gpu.ebo));
		if (gpu.vbo != 0) glDeleteBuffers(1, reinterpret_cast<const GLuint*>(&gpu.vbo));
		if (gpu.vao != 0) glDeleteVertexArrays(1, reinterpret_cast<const GLuint*>(&gpu.vao));
	}
	m_MeshCache.clear();
}

void RenderSystem::DestroyAllShaders()
{
	for (auto& [path, shader] : m_ShaderCache)
	{
		(void)path;
		if (shader.handle != 0)
		{
			glDeleteProgram((GLuint)shader.handle);
		}
	}
	m_ShaderCache.clear();
}

void RenderSystem::DestroyAllTextures()
{
	for (auto& [path, tex] : m_TextureCache)
	{
		(void)path;
		if (tex.handle != 0)
		{
			glDeleteTextures(1, reinterpret_cast<const GLuint*>(&tex.handle));
		}
	}
	m_TextureCache.clear();
}

bool RenderSystem::EnsureShaderLoaded(const std::string& shaderPath, EZ::u32& outProgram)
{
	auto it = m_ShaderCache.find(shaderPath);
	if (it != m_ShaderCache.end())
	{
		outProgram = it->second.handle;
		return outProgram != 0;
	}

	ParsedShaderSource parsed{};
	if (!ParseShaderFile(shaderPath, parsed))
	{
		return false;
	}

	const EZ::u32 vs = CompileShader(GL_VERTEX_SHADER, parsed.vertex.c_str());
	if (vs == 0)
	{
		return false;
	}

	const EZ::u32 fs = CompileShader(GL_FRAGMENT_SHADER, parsed.fragment.c_str());
	if (fs == 0)
	{
		glDeleteShader((GLuint)vs);
		return false;
	}

	const EZ::u32 program = LinkProgram(vs, fs);

	glDeleteShader((GLuint)vs);
	glDeleteShader((GLuint)fs);

	if (program == 0)
	{
		return false;
	}

	m_ShaderCache.emplace(shaderPath, ShaderProgram{ program });
	outProgram = program;
	return true;
}

bool RenderSystem::EnsureTextureLoaded(const std::string& imagePath, EZ::u32& outTexture)
{
	if (imagePath.empty())
	{
		outTexture = 0;
		return false;
	}

	auto it = m_TextureCache.find(imagePath);
	if (it != m_TextureCache.end())
	{
		outTexture = it->second.handle;
		return outTexture != 0;
	}

	int width = 0;
	int height = 0;
	int channels = 0;
	stbi_uc* pixels = stbi_load(imagePath.c_str(), &width, &height, &channels, 4);
	if (!pixels)
	{
		std::printf("[RenderSystem] Failed to load texture: %s\n", imagePath.c_str());
		outTexture = 0;
		return false;
	}

	GLuint tex = 0;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA8,
		width,
		height,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		pixels);

	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(pixels);

	GPUTexture gpu{};
	gpu.handle = (EZ::u32)tex;
	gpu.width = (EZ::u32)width;
	gpu.height = (EZ::u32)height;

	m_TextureCache.emplace(imagePath, gpu);
	outTexture = gpu.handle;
	return true;
}

bool RenderSystem::EnsureMeshUploaded(const DataProtocol::MeshAsset* meshAsset)
{
	if (!meshAsset)
	{
		return false;
	}

	if (m_MeshCache.find(meshAsset) != m_MeshCache.end())
	{
		return true;
	}

	if (meshAsset->vertexRawData.empty() || meshAsset->indexRawData.empty())
	{
		return false;
	}

	GLuint vao = 0;
	GLuint vbo = 0;
	GLuint ebo = 0;

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(
		GL_ARRAY_BUFFER,
		(GLsizeiptr)meshAsset->vertexRawData.size(),
		meshAsset->vertexRawData.data(),
		GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(
		GL_ELEMENT_ARRAY_BUFFER,
		(GLsizeiptr)meshAsset->indexRawData.size(),
		meshAsset->indexRawData.data(),
		GL_STATIC_DRAW);

	bool hasPosition = false;

	for (const auto& attr : meshAsset->vertexLayout.attributes)
	{
		GLenum glType = ToGLScalarType(attr.scalarType);
		if (glType == 0)
		{
			continue;
		}

		GLuint location = 99;
		switch (attr.semantic)
		{
		case DataProtocol::VertexSemantic::Position:    location = 0; hasPosition = true; break;
		case DataProtocol::VertexSemantic::Normal:      location = 1; break;
		case DataProtocol::VertexSemantic::UV:          location = 2; break;
		case DataProtocol::VertexSemantic::BoneIndices: location = 3; break;
		case DataProtocol::VertexSemantic::BoneWeights: location = 4; break;
		default: continue;
		}

		glEnableVertexAttribArray(location);

		const void* ptr = reinterpret_cast<const void*>(static_cast<uintptr_t>(attr.offset));

		if (attr.semantic == DataProtocol::VertexSemantic::BoneIndices)
		{
			glVertexAttribIPointer(
				location,
				(GLint)attr.componentCount,
				glType,
				(GLsizei)meshAsset->vertexLayout.stride,
				ptr);
		}
		else
		{
			glVertexAttribPointer(
				location,
				(GLint)attr.componentCount,
				glType,
				attr.normalized ? GL_TRUE : GL_FALSE,
				(GLsizei)meshAsset->vertexLayout.stride,
				ptr);
		}
	}

	glBindVertexArray(0);

	if (!hasPosition)
	{
		glDeleteBuffers(1, &ebo);
		glDeleteBuffers(1, &vbo);
		glDeleteVertexArrays(1, &vao);
		return false;
	}

	GPUMesh gpu{};
	gpu.vao = vao;
	gpu.vbo = vbo;
	gpu.ebo = ebo;
	gpu.indexGLType = (meshAsset->indexType == DataProtocol::IndexScalarType::UInt16)
		? GL_UNSIGNED_SHORT
		: GL_UNSIGNED_INT;

	m_MeshCache.emplace(meshAsset, gpu);
	return true;
}

bool RenderSystem::PrepareFrameData(EZ::WorldContext& world)
{
	if (m_FrameDataValid)
	{
		return m_FrameData.camera.valid;
	}

	return BuildFrameData(world);
}

bool RenderSystem::BuildFrameData(EZ::WorldContext& world)
{
	auto* window = world.GetPrimaryWindow();
	if (!window)
	{
		m_FrameData.Clear();
		m_FrameDataValid = false;
		return false;
	}

	const DataProtocol::UVec2 drawableSize = window->GetDrawableSize();
	if (drawableSize.x == 0 || drawableSize.y == 0)
	{
		m_FrameData.Clear();
		m_FrameDataValid = false;
		return false;
	}

	m_FrameData.Clear();
	m_FrameData.drawableSize = drawableSize;

	if (!BuildCameraFrameData(world, m_FrameData.camera))
	{
		m_FrameDataValid = false;
		return false;
	}

	BuildMainLightFrameData(world, m_FrameData.mainLight);
	CollectStaticRenderables(world, m_FrameData);
	CollectSkinnedRenderables(world, m_FrameData);

	m_FrameDataValid = true;
	return true;
}

bool RenderSystem::BuildCameraFrameData(EZ::WorldContext& world, RenderCameraFrameData& outCamera) const
{
	auto* window = world.GetPrimaryWindow();
	auto* entityManager = world.TryGet<ControlProtocol::EntityManager>();
	if (!window || !entityManager)
	{
		return false;
	}

	const DataProtocol::UVec2 drawableSize = window->GetDrawableSize();
	if (drawableSize.x == 0 || drawableSize.y == 0)
	{
		return false;
	}

	glm::vec3 cameraPos(8.0f, 6.0f, 8.0f);
	glm::vec3 cameraForward = glm::normalize(glm::vec3(-8.0f, -5.0f, -8.0f));

	BaseProtocol::CameraProjectionType projectionType = BaseProtocol::CameraProjectionType::Perspective;
	float fovYDegrees = 60.0f;
	float orthoSize = 10.0f;
	float nearClip = 0.1f;
	float farClip = 1000.0f;
	DataProtocol::Vec4 clearColor{ 0.08f, 0.10f, 0.14f, 1.0f };

	entityManager->ForEach<BaseProtocol::CameraComponent, BaseProtocol::LocalToWorld>(
		[&](EZ::Entity entity, BaseProtocol::CameraComponent& camera, BaseProtocol::LocalToWorld& localToWorld)
		{
			(void)entity;

			if (!camera.isMainCamera)
			{
				return;
			}

			cameraPos = ExtractTranslation(localToWorld.value);
			cameraForward = ExtractForward(localToWorld.value);

			projectionType = camera.projectionType;
			fovYDegrees = camera.fovYDegrees;
			orthoSize = camera.orthoSize;
			nearClip = camera.nearClip;
			farClip = camera.farClip;

			if (camera.clearColorEnabled)
			{
				clearColor = camera.clearColor;
			}
		});

	const glm::mat4 view = glm::lookAt(
		cameraPos,
		cameraPos + cameraForward,
		glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 proj(1.0f);
	const float aspect = static_cast<float>(drawableSize.x) / static_cast<float>(drawableSize.y);

	if (projectionType == BaseProtocol::CameraProjectionType::Orthographic)
	{
		const float halfHeight = orthoSize;
		const float halfWidth = halfHeight * aspect;
		proj = glm::ortho(
			-halfWidth,
			halfWidth,
			-halfHeight,
			halfHeight,
			nearClip,
			farClip);
	}
	else
	{
		proj = glm::perspective(
			glm::radians(fovYDegrees),
			aspect,
			nearClip,
			farClip);
	}

	const glm::mat4 viewProj = proj * view;

	std::memcpy(outCamera.view, &view[0][0], sizeof(outCamera.view));
	std::memcpy(outCamera.proj, &proj[0][0], sizeof(outCamera.proj));
	std::memcpy(outCamera.viewProj, &viewProj[0][0], sizeof(outCamera.viewProj));

	outCamera.cameraPos[0] = cameraPos.x;
	outCamera.cameraPos[1] = cameraPos.y;
	outCamera.cameraPos[2] = cameraPos.z;
	outCamera.clearColor = clearColor;
	outCamera.valid = true;
	return true;
}

void RenderSystem::BuildMainLightFrameData(EZ::WorldContext& world, RenderMainLightFrameData& outLight) const
{
	auto* entityManager = world.TryGet<ControlProtocol::EntityManager>();
	if (!entityManager)
	{
		return;
	}

	entityManager->ForEach<BaseProtocol::LightComponent, BaseProtocol::LocalToWorld>(
		[&](EZ::Entity entity, BaseProtocol::LightComponent& light, BaseProtocol::LocalToWorld& localToWorld)
		{
			(void)entity;

			if (light.type != BaseProtocol::LightType::Directional)
			{
				return;
			}

			glm::vec3 dir = ExtractForward(localToWorld.value);
			outLight.lightDir[0] = dir.x;
			outLight.lightDir[1] = dir.y;
			outLight.lightDir[2] = dir.z;

			outLight.lightColor[0] = light.color.x * light.intensity;
			outLight.lightColor[1] = light.color.y * light.intensity;
			outLight.lightColor[2] = light.color.z * light.intensity;
			outLight.valid = true;
		});
}

void RenderSystem::CollectStaticRenderables(EZ::WorldContext& world, RenderFrameData& frameData) const
{
	auto* entityManager = world.TryGet<ControlProtocol::EntityManager>();
	if (!entityManager)
	{
		return;
	}

	entityManager->ForEach<BaseProtocol::MeshRendererComponent, BaseProtocol::LocalToWorld>(
		[&](EZ::Entity entity, BaseProtocol::MeshRendererComponent& renderer, BaseProtocol::LocalToWorld& localToWorld)
		{
			(void)entity;

			if (!renderer.visible || !renderer.meshAsset)
			{
				return;
			}
			if (renderer.meshAsset->hasSkinning)
			{
				return;
			}

			for (EZ::u32 subMeshIndex = 0; subMeshIndex < (EZ::u32)renderer.meshAsset->subMeshes.size(); ++subMeshIndex)
			{
				const auto& sub = renderer.meshAsset->subMeshes[subMeshIndex];

				const DataProtocol::MaterialAsset* material = nullptr;
				if (sub.materialSlot < renderer.materialAssets.size())
				{
					material = renderer.materialAssets[sub.materialSlot];
				}

				StaticRenderableFrameItem item{};
				item.meshAsset = renderer.meshAsset;
				item.materialAsset = material;
				item.localToWorld = &localToWorld.value;
				item.subMeshIndex = subMeshIndex;
				item.castShadow = renderer.castShadow;
				item.receiveShadow = renderer.receiveShadow;

				frameData.staticRenderables.push_back(item);
			}
		});
}

void RenderSystem::CollectSkinnedRenderables(EZ::WorldContext& world, RenderFrameData& frameData) const
{
	auto* entityManager = world.TryGet<ControlProtocol::EntityManager>();
	if (!entityManager)
	{
		return;
	}

	static int InstanceConst = 0;

	entityManager->ForEach<
		BaseProtocol::SkinnedMeshRendererComponent,
		BaseProtocol::SkinningPaletteComponent,
		BaseProtocol::LocalToWorld
	>(
		[&](EZ::Entity entity,
			BaseProtocol::SkinnedMeshRendererComponent& renderer,
			BaseProtocol::SkinningPaletteComponent& palette,
			BaseProtocol::LocalToWorld& localToWorld)
		{
			(void)entity;

			if (!renderer.visible || !renderer.meshAsset || !renderer.skeletonAsset || !renderer.skinAsset)
			{
				return;
			}

			InstanceConst++;

			const auto& finalMatrices = palette.GetFinalMatrices();
			if (finalMatrices.empty())
			{
				return;
			}
			
			m_FrameStats.skinnedInstanceCount += 1;

			for (EZ::u32 subMeshIndex = 0; subMeshIndex < (EZ::u32)renderer.meshAsset->subMeshes.size(); ++subMeshIndex)
			{
				const auto& sub = renderer.meshAsset->subMeshes[subMeshIndex];

				const DataProtocol::MaterialAsset* material = nullptr;
				if (sub.materialSlot < renderer.materialAssets.size())
				{
					material = renderer.materialAssets[sub.materialSlot];
				}

				SkinnedRenderableFrameItem item{};
				item.meshAsset = renderer.meshAsset;
				item.skeletonAsset = renderer.skeletonAsset;
				item.skinAsset = renderer.skinAsset;
				item.finalMatrices = &finalMatrices;
				item.materialAsset = material;
				item.localToWorld = &localToWorld.value;
				item.subMeshIndex = subMeshIndex;
				item.castShadow = renderer.castShadow;
				item.receiveShadow = renderer.receiveShadow;

				frameData.skinnedRenderables.push_back(item);
			}
		});
}

bool RenderSystem::SetupPersistentFrameTarget(
	EZ::RenderImageTag tag,
	const EZ::RenderImageDesc& desc,
	ControlProtocol::RenderImageBuffer& imageBuffer,
	ControlProtocol::RenderDeviceController& device)
{
	auto& slot = imageBuffer.GetOrCreate(tag);

	bool needRecreate = false;

	if (!slot.desc.valid)
	{
		needRecreate = true;
	}
	else
	{
		needRecreate =
			slot.desc.extent.width != desc.extent.width ||
			slot.desc.extent.height != desc.extent.height ||
			slot.desc.format != desc.format ||
			slot.desc.imported != desc.imported;
	}

	if (needRecreate)
	{
		device.DestroyRenderTarget(slot);
		device.DestroyImage(slot);
	}

	slot.desc = desc;
	slot.desc.tag = tag;
	slot.desc.valid = true;
	slot.desc.persistent = true;

	return device.EnsureImage(slot);
}

bool RenderSystem::SetupForwardOpaqueTargets(
	EZ::WorldContext& world,
	ControlProtocol::RenderImageBuffer& imageBuffer,
	ControlProtocol::RenderDeviceController& device)
{
	if (!PrepareFrameData(world))
	{
		return false;
	}

	EZ::RenderImageDesc colorDesc{};
	colorDesc.source = EZ::RenderSourceType::ForwardPass;
	colorDesc.extent.width = m_FrameData.drawableSize.x;
	colorDesc.extent.height = m_FrameData.drawableSize.y;
	colorDesc.format = EZ::RenderFormat::RGBA8_UNorm;
	colorDesc.valid = true;
	colorDesc.imported = false;
	colorDesc.persistent = true;
	colorDesc.sampled = true;
	colorDesc.colorAttachment = true;
	colorDesc.depthStencilAttachment = false;
	colorDesc.loadAction = EZ::RenderLoadAction::Clear;
	colorDesc.storeAction = EZ::RenderStoreAction::Store;
	colorDesc.clearValue.color.r = m_FrameData.camera.clearColor.x;
	colorDesc.clearValue.color.g = m_FrameData.camera.clearColor.y;
	colorDesc.clearValue.color.b = m_FrameData.camera.clearColor.z;
	colorDesc.clearValue.color.a = m_FrameData.camera.clearColor.w;

	EZ::RenderImageDesc depthDesc{};
	depthDesc.source = EZ::RenderSourceType::ForwardPass;
	depthDesc.extent.width = m_FrameData.drawableSize.x;
	depthDesc.extent.height = m_FrameData.drawableSize.y;
	depthDesc.format = EZ::RenderFormat::D24_UNorm_S8_UInt;
	depthDesc.valid = true;
	depthDesc.imported = false;
	depthDesc.persistent = true;
	depthDesc.sampled = false;
	depthDesc.colorAttachment = false;
	depthDesc.depthStencilAttachment = true;
	depthDesc.loadAction = EZ::RenderLoadAction::Clear;
	depthDesc.storeAction = EZ::RenderStoreAction::Store;
	depthDesc.clearValue.depthStencil.depth = 1.0f;
	depthDesc.clearValue.depthStencil.stencil = 0;

	const bool okColor = SetupPersistentFrameTarget(
		EZ::RenderImageTag::ForwardOpaque,
		colorDesc,
		imageBuffer,
		device);

	const bool okDepth = SetupPersistentFrameTarget(
		EZ::RenderImageTag::SceneDepth,
		depthDesc,
		imageBuffer,
		device);

	return okColor && okDepth;
}

void RenderSystem::RenderStaticPass(
	const std::vector<StaticRenderableFrameItem>& renderables,
	const RenderCameraFrameData& camera,
	const RenderMainLightFrameData& light)
{
	std::unordered_map<StaticBatchKey, StaticBatch, StaticBatchKeyHash> batches;

	for (const auto& item : renderables)
	{
		if (!item.meshAsset || !item.localToWorld)
		{
			continue;
		}

		if (!EnsureMeshUploaded(item.meshAsset))
		{
			continue;
		}

		std::string shaderPath = "Project/Test01/res/shaders/Lit/StaticLit.shader";
		if (item.materialAsset && !item.materialAsset->shaderPath.empty())
		{
			shaderPath = item.materialAsset->shaderPath;
		}

		EZ::u32 program = 0;
		if (!EnsureShaderLoaded(shaderPath, program))
		{
			continue;
		}

		StaticBatchKey key{};
		key.meshAsset = item.meshAsset;
		key.materialAsset = item.materialAsset;
		key.shaderProgram = program;
		key.subMeshIndex = item.subMeshIndex;

		auto& batch = batches[key];
		batch.meshAsset = item.meshAsset;
		batch.materialAsset = item.materialAsset;
		batch.shaderProgram = program;
		batch.subMeshIndex = item.subMeshIndex;
		batch.modelMatrices.push_back(item.localToWorld);
	}

	for (auto& [key, batch] : batches)
	{
		(void)key;

		auto meshIt = m_MeshCache.find(batch.meshAsset);
		if (meshIt == m_MeshCache.end())
		{
			continue;
		}

		const auto& gpuMesh = meshIt->second;
		const auto& sub = batch.meshAsset->subMeshes[batch.subMeshIndex];

		m_InstanceUploadCache.resize(batch.modelMatrices.size());
		for (size_t i = 0; i < batch.modelMatrices.size(); ++i)
		{
			std::memcpy(m_InstanceUploadCache[i].model, batch.modelMatrices[i]->m.data(), sizeof(float) * 16);
		}

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, (GLuint)m_InstanceSSBO);
		glBufferData(
			GL_SHADER_STORAGE_BUFFER,
			(GLsizeiptr)(m_InstanceUploadCache.size() * sizeof(InstanceData)),
			m_InstanceUploadCache.data(),
			GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, (GLuint)m_InstanceSSBO);

		glUseProgram((GLuint)batch.shaderProgram);

		glUniformMatrix4fv(glGetUniformLocation((GLuint)batch.shaderProgram, "u_ViewProj"), 1, GL_FALSE, camera.viewProj);
		glUniform3fv(glGetUniformLocation((GLuint)batch.shaderProgram, "u_LightDir"), 1, light.lightDir);
		glUniform3fv(glGetUniformLocation((GLuint)batch.shaderProgram, "u_LightColor"), 1, light.lightColor);

		const float ambient[3] = { 0.16f, 0.18f, 0.20f };
		glUniform3fv(glGetUniformLocation((GLuint)batch.shaderProgram, "u_AmbientColor"), 1, ambient);

		float baseColor[4] = { 1, 1, 1, 1 };
		EZ::u32 baseColorTex = 0;
		int hasBaseColorTex = 0;

		if (batch.materialAsset)
		{
			baseColor[0] = batch.materialAsset->baseColor.x;
			baseColor[1] = batch.materialAsset->baseColor.y;
			baseColor[2] = batch.materialAsset->baseColor.z;
			baseColor[3] = batch.materialAsset->baseColor.w;

			if (const char* imagePath = FindTextureSlot(batch.materialAsset, "BaseColor"))
			{
				if (EnsureTextureLoaded(imagePath, baseColorTex))
				{
					hasBaseColorTex = 1;
				}
			}

			if (batch.materialAsset->depthTest) glEnable(GL_DEPTH_TEST);
			else glDisable(GL_DEPTH_TEST);

			glDepthMask(batch.materialAsset->depthWrite ? GL_TRUE : GL_FALSE);

			if (batch.materialAsset->doubleSided || batch.materialAsset->cullMode == DataProtocol::MaterialCullMode::None)
			{
				glDisable(GL_CULL_FACE);
			}
			else
			{
				glEnable(GL_CULL_FACE);
				glCullFace(batch.materialAsset->cullMode == DataProtocol::MaterialCullMode::Front ? GL_FRONT : GL_BACK);
			}
		}
		else
		{
			glEnable(GL_DEPTH_TEST);
			glDepthMask(GL_TRUE);
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
		}

		glUniform4fv(glGetUniformLocation((GLuint)batch.shaderProgram, "u_BaseColor"), 1, baseColor);
		glUniform1i(glGetUniformLocation((GLuint)batch.shaderProgram, "u_HasBaseColorTex"), hasBaseColorTex);
		glUniform1i(glGetUniformLocation((GLuint)batch.shaderProgram, "u_BaseColorTex"), 0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, hasBaseColorTex ? (GLuint)baseColorTex : 0);

		glBindVertexArray((GLuint)gpuMesh.vao);
		glDrawElementsInstanced(
			GL_TRIANGLES,
			(GLsizei)sub.indexCount,
			(GLenum)gpuMesh.indexGLType,
			reinterpret_cast<const void*>(static_cast<uintptr_t>(
				sub.indexOffset* (batch.meshAsset->indexType == DataProtocol::IndexScalarType::UInt16 ? 2 : 4))),
			(GLsizei)batch.modelMatrices.size());

		m_FrameStats.drawCalls += 1;
		m_FrameStats.staticDrawCalls += 1;
		m_FrameStats.staticInstanceCount += static_cast<EZ::u32>(batch.modelMatrices.size());

		glBindVertexArray(0);
	}
}

void RenderSystem::RenderSkinnedPass(
	const std::vector<SkinnedRenderableFrameItem>& renderables,
	const RenderCameraFrameData& camera,
	const RenderMainLightFrameData& light)
{
	std::unordered_map<SkinnedBatchKey, SkinnedBatch, SkinnedBatchKeyHash> batches;

	for (const auto& item : renderables)
	{
		if (!item.meshAsset || !item.skeletonAsset || !item.skinAsset || !item.finalMatrices || !item.localToWorld)
		{
			continue;
		}

		if (item.finalMatrices->empty())
		{
			continue;
		}

		if (!EnsureMeshUploaded(item.meshAsset))
		{
			continue;
		}

		std::string shaderPath = "Project/Test01/res/shaders/Lit/CharacterLit.shader";
		if (item.materialAsset && !item.materialAsset->shaderPath.empty())
		{
			shaderPath = item.materialAsset->shaderPath;
		}

		EZ::u32 program = 0;
		if (!EnsureShaderLoaded(shaderPath, program))
		{
			continue;
		}

		SkinnedBatchKey key{};
		key.meshAsset = item.meshAsset;
		key.materialAsset = item.materialAsset;
		key.skeletonAsset = item.skeletonAsset;
		key.skinAsset = item.skinAsset;
		key.shaderProgram = program;
		key.subMeshIndex = item.subMeshIndex;

		auto& batch = batches[key];
		batch.meshAsset = item.meshAsset;
		batch.materialAsset = item.materialAsset;
		batch.skeletonAsset = item.skeletonAsset;
		batch.skinAsset = item.skinAsset;
		batch.shaderProgram = program;
		batch.subMeshIndex = item.subMeshIndex;
		batch.instances.push_back({ item.localToWorld, item.finalMatrices });
	}

	for (auto& [key, batch] : batches)
	{
		(void)key;

		if (!batch.meshAsset || !batch.skinAsset || batch.instances.empty())
		{
			continue;
		}

		auto meshIt = m_MeshCache.find(batch.meshAsset);
		if (meshIt == m_MeshCache.end())
		{
			continue;
		}

		const auto& gpuMesh = meshIt->second;
		const auto& sub = batch.meshAsset->subMeshes[batch.subMeshIndex];

		m_InstanceUploadCache.resize(batch.instances.size());
		m_BonePaletteUploadCache.clear();

		for (size_t i = 0; i < batch.instances.size(); ++i)
		{
			const auto& inst = batch.instances[i];

			std::memcpy(
				m_InstanceUploadCache[i].model,
				inst.modelMatrix->m.data(),
				sizeof(float) * 16);

			m_InstanceUploadCache[i].paletteBaseOffset = 0;
			m_InstanceUploadCache[i].padding0 = 0;
			m_InstanceUploadCache[i].padding1 = 0;
			m_InstanceUploadCache[i].padding2 = 0;

			m_InstanceUploadCache[i].paletteBaseOffset =
				static_cast<EZ::u32>(m_BonePaletteUploadCache.size());

			m_BonePaletteUploadCache.insert(
				m_BonePaletteUploadCache.end(),
				inst.finalMatrices->begin(),
				inst.finalMatrices->end());
		}

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, (GLuint)m_InstanceSSBO);
		glBufferData(
			GL_SHADER_STORAGE_BUFFER,
			(GLsizeiptr)(m_InstanceUploadCache.size() * sizeof(InstanceData)),
			m_InstanceUploadCache.data(),
			GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, (GLuint)m_InstanceSSBO);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, (GLuint)m_BoneSSBO);
		glBufferData(
			GL_SHADER_STORAGE_BUFFER,
			(GLsizeiptr)(m_BonePaletteUploadCache.size() * sizeof(DataProtocol::Mat4)),
			m_BonePaletteUploadCache.data(),
			GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, (GLuint)m_BoneSSBO);

		glUseProgram((GLuint)batch.shaderProgram);

		glUniformMatrix4fv(glGetUniformLocation((GLuint)batch.shaderProgram, "u_ViewProj"), 1, GL_FALSE, camera.viewProj);
		glUniform3fv(glGetUniformLocation((GLuint)batch.shaderProgram, "u_LightDir"), 1, light.lightDir);
		glUniform3fv(glGetUniformLocation((GLuint)batch.shaderProgram, "u_LightColor"), 1, light.lightColor);

		const float ambient[3] = { 0.16f, 0.18f, 0.20f };
		glUniform3fv(glGetUniformLocation((GLuint)batch.shaderProgram, "u_AmbientColor"), 1, ambient);
		glUniform3fv(glGetUniformLocation((GLuint)batch.shaderProgram, "u_CameraPos"), 1, camera.cameraPos);

		const float rimColor[3] = { 0.15f, 0.18f, 0.22f };
		glUniform3fv(glGetUniformLocation((GLuint)batch.shaderProgram, "u_RimColor"), 1, rimColor);
		glUniform1f(glGetUniformLocation((GLuint)batch.shaderProgram, "u_RimPower"), 2.0f);

		float baseColor[4] = { 1, 1, 1, 1 };
		EZ::u32 baseColorTex = 0;
		int hasBaseColorTex = 0;

		if (batch.materialAsset)
		{
			baseColor[0] = batch.materialAsset->baseColor.x;
			baseColor[1] = batch.materialAsset->baseColor.y;
			baseColor[2] = batch.materialAsset->baseColor.z;
			baseColor[3] = batch.materialAsset->baseColor.w;

			if (const char* imagePath = FindTextureSlot(batch.materialAsset, "BaseColor"))
			{
				if (EnsureTextureLoaded(imagePath, baseColorTex))
				{
					hasBaseColorTex = 1;
				}
			}

			if (batch.materialAsset->depthTest) glEnable(GL_DEPTH_TEST);
			else glDisable(GL_DEPTH_TEST);

			glDepthMask(batch.materialAsset->depthWrite ? GL_TRUE : GL_FALSE);

			if (batch.materialAsset->doubleSided || batch.materialAsset->cullMode == DataProtocol::MaterialCullMode::None)
			{
				glDisable(GL_CULL_FACE);
			}
			else
			{
				glEnable(GL_CULL_FACE);
				glCullFace(batch.materialAsset->cullMode == DataProtocol::MaterialCullMode::Front ? GL_FRONT : GL_BACK);
			}
		}
		else
		{
			glEnable(GL_DEPTH_TEST);
			glDepthMask(GL_TRUE);
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
		}

		glUniform4fv(glGetUniformLocation((GLuint)batch.shaderProgram, "u_BaseColor"), 1, baseColor);
		glUniform1i(glGetUniformLocation((GLuint)batch.shaderProgram, "u_HasBaseColorTex"), hasBaseColorTex);
		glUniform1i(glGetUniformLocation((GLuint)batch.shaderProgram, "u_BaseColorTex"), 0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, hasBaseColorTex ? (GLuint)baseColorTex : 0);

		glBindVertexArray((GLuint)gpuMesh.vao);
		glDrawElementsInstanced(
			GL_TRIANGLES,
			(GLsizei)sub.indexCount,
			(GLenum)gpuMesh.indexGLType,
			reinterpret_cast<const void*>(static_cast<uintptr_t>(
				sub.indexOffset * (batch.meshAsset->indexType == DataProtocol::IndexScalarType::UInt16 ? 2 : 4))),
			(GLsizei)batch.instances.size());

		m_FrameStats.drawCalls += 1;
		m_FrameStats.skinnedDrawCalls += 1;
		glBindVertexArray(0);
	}
}
void RenderSystem::ExecuteForwardOpaque(
	EZ::WorldContext& world,
	ControlProtocol::RenderImageBuffer& imageBuffer,
	ControlProtocol::RenderDeviceController& device)
{
	if (!PrepareFrameData(world))
	{
		return;
	}

	auto* colorSlot = imageBuffer.TryGet(EZ::RenderImageTag::ForwardOpaque);
	auto* depthSlot = imageBuffer.TryGet(EZ::RenderImageTag::SceneDepth);
	if (!colorSlot || !depthSlot)
	{
		return;
	}

	if (colorSlot->desc.loadAction == EZ::RenderLoadAction::Clear)
	{
		device.ClearColor(*colorSlot, colorSlot->desc.clearValue.color);
	}

	if (depthSlot->desc.loadAction == EZ::RenderLoadAction::Clear)
	{
		device.ClearDepth(*depthSlot, depthSlot->desc.clearValue.depthStencil);
	}

	if (!device.BeginPass(colorSlot, depthSlot))
	{
		return;
	}

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	RenderStaticPass(
		m_FrameData.staticRenderables,
		m_FrameData.camera,
		m_FrameData.mainLight);

	RenderSkinnedPass(
		m_FrameData.skinnedRenderables,
		m_FrameData.camera,
		m_FrameData.mainLight);

	device.EndPass();
}

bool RenderSystem::Present(
	EZ::WorldContext& world,
	ControlProtocol::RenderImageBuffer& imageBuffer,
	ControlProtocol::RenderDeviceController& device)
{
	auto* window = world.GetPrimaryWindow();
	if (!window)
	{
		return false;
	}

	const auto* finalColor = imageBuffer.TryGet(EZ::RenderImageTag::FinalColor);
	const auto* forwardOpaque = imageBuffer.TryGet(EZ::RenderImageTag::ForwardOpaque);

	if (finalColor)
	{
		return device.Present(*window, *finalColor);
	}

	if (forwardOpaque)
	{
		return device.Present(*window, *forwardOpaque);
	}

	return false;
}

void RenderSystem::EndFrame(EZ::ProjectContext& project, EZ::WorldContext& world, float deltaTime)
{
	(void)project;
	(void)world;
	(void)deltaTime;
}