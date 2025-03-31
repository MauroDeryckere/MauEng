#ifndef MAUENG_GLFW_WINDOW_H
#define MAUENG_GLFW_WINDOW_H

// Currently I only aim to support a single window (glfw) so there is no need to use another fancier yet than a simple struct
// The same goes for rendering, we only care about vulkan currently, this could be made more modular in the fure

#include "EnginePCH.h"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

namespace MauEng
{

	struct GLFWWindow final
	{
		GLFWwindow* window{ nullptr };
		uint16_t width{ 1280 };
		uint16_t height{ 720 };

		std::string windowTitle{ "VulkanProject" };

		static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);

		GLFWWindow();
		~GLFWWindow();

		// Renderer must be registered 1st! 
		void Initialize();


		GLFWWindow(GLFWWindow const&) = delete;
		GLFWWindow(GLFWWindow&&) = delete;
		GLFWWindow& operator=(GLFWWindow const&) = delete;
		GLFWWindow& operator=(GLFWWindow&&) = delete;
	};
}

#endif // MAUENG_GLFW_WINDOW_H