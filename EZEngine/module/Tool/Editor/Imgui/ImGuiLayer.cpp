#include "Tool/Editor/ImGui/ImGuiLayer.h"

#include <SDL3/SDL.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_sdl3.h"

#include "core/Context/RunTimeContext/WorldContext.h"

namespace Tool
{
	bool ImGuiLayer::Initialize(EZ::WorldContext& world)
	{
		if (m_Initialized)
		{
			return true;
		}

		m_Window = world.GetPrimaryWindow();
		if (!m_Window)
		{
			return false;
		}

		SDL_Window* sdlWindow = static_cast<SDL_Window*>(m_Window->GetBackendWindowHandle());
		if (!sdlWindow)
		{
			return false;
		}

		void* glContext = SDL_GL_GetCurrentContext();
		if (!glContext)
		{
			return false;
		}

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

		ImGui::StyleColorsDark();

		if (!ImGui_ImplSDL3_InitForOpenGL(sdlWindow, glContext))
		{
			ImGui::DestroyContext();
			return false;
		}

		if (!ImGui_ImplOpenGL3_Init("#version 410"))
		{
			ImGui_ImplSDL3_Shutdown();
			ImGui::DestroyContext();
			return false;
		}

		m_RawEventHandle = ControlProtocol::WindowController::AddRawEventListener(
			&ImGuiLayer::RawEventBridge,
			this);

		if (!m_RawEventHandle.IsValid())
		{
			ImGui_ImplOpenGL3_Shutdown();
			ImGui_ImplSDL3_Shutdown();
			ImGui::DestroyContext();
			return false;
		}

		m_Initialized = true;
		m_FrameBegun = false;
		return true;
	}

	void ImGuiLayer::Shutdown()
	{
		if (!m_Initialized)
		{
			return;
		}

		if (m_RawEventHandle.IsValid())
		{
			ControlProtocol::WindowController::RemoveRawEventListener(m_RawEventHandle);
			m_RawEventHandle = {};
		}

		m_FrameBegun = false;

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplSDL3_Shutdown();
		ImGui::DestroyContext();

		m_Window = nullptr;
		m_Initialized = false;
	}

	void ImGuiLayer::BeginFrame()
	{
		if (!m_Initialized || m_FrameBegun)
		{
			return;
		}

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();

		m_FrameBegun = true;
	}

	void ImGuiLayer::Render()
	{
		if (!m_Initialized || !m_FrameBegun)
		{
			return;
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		m_FrameBegun = false;
	}

	void ImGuiLayer::RawEventBridge(const void* backendEvent, void* userData)
	{
		(void)userData;

		if (!backendEvent)
		{
			return;
		}

		const SDL_Event* sdlEvent = static_cast<const SDL_Event*>(backendEvent);
		ImGui_ImplSDL3_ProcessEvent(sdlEvent);
	}
}