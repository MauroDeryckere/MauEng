#include "ImGUILayer.h"

#include "Window/SDLWindow.h"

#include "InternalServiceLocator.h"

#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_vulkan.h"

#include "Components/CDebugText.h"

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
	}

	void ImGUILayer::Render(class Scene* scene, SDLWindow* pWindow)
	{
		ImGuiWindowFlags constexpr flags
		{
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoBackground |
			ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_NoBringToFrontOnFocus
		};

		ImGuiViewport const* viewport{ ImGui::GetMainViewport() };
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

		ImGui::Begin("DockSpaceBackground", nullptr, flags);
			ImGui::PopStyleVar(2);

			ImGuiID const dockSpaceID{ ImGui::GetID("MyDockSpace") };
			ImGui::DockSpace(dockSpaceID, ImVec2{ 0.0f, 0.0f }, ImGuiDockNodeFlags_PassthruCentralNode);

			{
				// Draw debug text
				ImDrawList* drawList{ ImGui::GetWindowDrawList() };
				ImVec2 const pos{ viewport->Pos };

				auto const* camera{ scene->GetCameraManager().GetActiveCamera() };

				glm::mat4 const view{ camera->GetViewMatrix() };
				glm::mat4 const proj{ camera->GetProjectionMatrix() };
				
				glm::mat4 const viewProj{ proj * view };
				ImFont* defaultFont{ ImGui::GetFont() };

				auto v{ scene->GetECSWorld().View<CDebugText, CTransform>() };
				v.Each([drawList, &pos, &viewProj, pWindow, defaultFont, &cameraPos = camera->GetPosition()](CDebugText const& d, CTransform const& t)
					{
						auto const res{ Camera::IsInFrustum(t.translation, viewProj, pWindow->width, pWindow->height) };
						if (res.first)
						{
							float const distance{ glm::distance(cameraPos, t.translation) };
							float const fontSize{ d.scaleWithDistance ? glm::clamp(d.baseFontSize / distance, d.minFontSize, d.maxFontSize)
																	  : d.baseFontSize };

							ImVec2 const textSize{ defaultFont->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, d.text.c_str()) };

							float offsetX{ 0.f };
							float offsetY{ 0.f};
							switch (d.hAlign)
							{
								case HorizontalAlignment::Left: offsetX = 0.f; break;
								case HorizontalAlignment::Center: offsetX = -textSize.x / 2.f; break;
								case HorizontalAlignment::Right: offsetX = -textSize.x; break;
							}

							switch (d.vAlign)
							{
								case VerticalAlignment::Top: offsetY = 0.f; break;
								case VerticalAlignment::Middle: offsetY = -textSize.y / 2.f; break;
								case VerticalAlignment::Bottom: offsetY = -textSize.y; break;
							}

							drawList->AddText(defaultFont, fontSize,ImVec2{ pos.x + res.second.x + offsetX, pos.y + res.second.y + offsetY }, IM_COL32(d.colour.r * 255.f, d.colour.g * 255.f, d.colour.b * 255.f, d.colour.a * 255.f), d.text.c_str());
						}
					});
			}

		ImGui::End();

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
