#include "ImGUILayer.h"

#include "Window/SDLWindow.h"

#include "InternalServiceLocator.h"

#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_vulkan.h"
namespace MauEng
{
	void ImGUILayer::Init(SDLWindow* pWindow)
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io{ ImGui::GetIO() };
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		io.DisplaySize = ImVec2{ static_cast<float>(pWindow->width), static_cast<float>(pWindow->height) };

		InternalServiceLocator::GetRenderer().InitImGUI();
	}

	void ImGUILayer::Destroy()
	{
		InternalServiceLocator::GetRenderer().DestroyImGUI();

		ImGui_ImplSDL3_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGUILayer::BeginFrame()
	{
		RENDERER.BeginImGUIFrame();

		// Create main dockspace window
		ImGuiWindowFlags windowFlags{ ImGuiWindowFlags_NoDocking };
		windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		windowFlags |= ImGuiWindowFlags_NoBackground;

		ImGuiViewport* viewport{ ImGui::GetMainViewport() };
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

		ImGui::Begin("DockSpaceBackground", nullptr, windowFlags);
		ImGui::PopStyleVar(2);

		// Use flags to prevent window splitting in center
		ImGuiID const dockspaceID{ ImGui::GetID("MyDockSpace") };
		ImGui::DockSpace(dockspaceID, ImVec2{ 0.0f, 0.0f }, ImGuiDockNodeFlags_PassthruCentralNode);

		ImGui::End();
	}

	void ImGUILayer::Render(MauEng::Camera const* cam)
	{
		// Temp test
		ImGui::Begin("Debug Info");
		ImGui::Text("Frame time: %.3f ms", 10.f);
		ImGui::End();
	}

	void ImGUILayer::EndFrame()
	{
		RENDERER.EndImGUIFrame();
	}
}
