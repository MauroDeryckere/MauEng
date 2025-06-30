#include "ImGUILayer.h"

#include "Window/SDLWindow.h"

#include "InternalServiceLocator.h"

#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_vulkan.h"

#include "Components/CDebugText.h"
#include "Logging/ImGUILogger.h"

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

			RenderDebugText(scene, pWindow);

		ImGui::End();

		// Temp test
		ImGui::Begin("Debug Info");
			ImGui::Text("Frame time: %.3f ms", 10.f);
		ImGui::End();

		RenderConsoleOutput();
	}

	void ImGUILayer::EndFrame()
	{
		RENDERER.EndImGUIFrame();
	}

	void ImGUILayer::RenderDebugText(class Scene* scene, SDLWindow* pWindow)
	{
		ImGuiViewport const* viewport{ ImGui::GetMainViewport() };

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
					float offsetY{ 0.f };
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

					drawList->AddText(defaultFont, fontSize, ImVec2{ pos.x + res.second.x + offsetX, pos.y + res.second.y + offsetY }, IM_COL32(d.colour.r * 255.f, d.colour.g * 255.f, d.colour.b * 255.f, d.colour.a * 255.f), d.text.c_str());
				}
			});
	}

	void ImGUILayer::RenderConsoleOutput()
	{
		if (auto* logger{ dynamic_cast<MauEng::ImGUILogger*>(&LOGGER) })
		{
			auto const& logBuffer{ logger->GetLogBuffer() };
			bool autoScroll{ logger->GetAutoScroll() };

			ImGui::Begin("Console Output");
				if (ImGui::Button("Clear"))
				{
					logger->Clear();
				}
				ImGui::SameLine();
				if (ImGui::Checkbox("Auto-scroll", &autoScroll))
				{
					logger->SetAutoScroll(autoScroll);
				}

				ImGui::Separator();

				ImGui::BeginChild("LogRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
					for (auto const& log : logBuffer)
					{
						ImVec4 color;
						switch (log.priority)
						{
							case MauCor::ELogPriority::Trace: color = ImVec4(0.5f, 0.5f, 0.5f, 1); break;  // Dim gray
							case MauCor::ELogPriority::Debug: color = ImVec4(0.3f, 0.7f, 1.0f, 1); break;  // Light blue
							case MauCor::ELogPriority::Info:  color = ImVec4(0.6f, 1.0f, 0.6f, 1); break;  // Soft green
							case MauCor::ELogPriority::Warn:  color = ImVec4(1.0f, 0.85f, 0.35f, 1); break; // Amber
							case MauCor::ELogPriority::Error: color = ImVec4(1.0f, 0.4f, 0.4f, 1); break;  // Light red
							case MauCor::ELogPriority::Fatal: color = ImVec4(1.0f, 0.1f, 0.1f, 1); break;  // Bright red
							default:                          color = ImVec4(1, 1, 1, 1); break;           // White
						}

						ImGui::PushStyleColor(ImGuiCol_Text, color);
						ImGui::Text("[%s] %s", log.category.c_str(), log.message.c_str());
						ImGui::PopStyleColor();
					}

					if (autoScroll)
						ImGui::SetScrollHereY(1.0f);
				ImGui::EndChild();
			ImGui::End();
		}
		else
		{
			ME_ASSERT(false);
		}
	}
}
