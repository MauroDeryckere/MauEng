#include <stdexcept>

#include "GlfwWindow.h"
#include "Renderer.h"

#include "GLFW/glfw3.h"

namespace MauEng
{
	void GLFWWindow::FramebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		//TODO when renderer is in service locator, no need to cast here (?)
		auto pRenderer { reinterpret_cast<MauRen::Renderer*>(glfwGetWindowUserPointer(window)) };
		pRenderer->ResizeWindow();

		//TODO change the width and height variable
	}

	GLFWWindow::GLFWWindow()
	{
		if (not (glfwInit() == GLFW_TRUE))
		{
			throw std::runtime_error("Failed to initialize GLFW!");
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		window = glfwCreateWindow(static_cast<int>(width), static_cast<int>(height), windowTitle.data(), nullptr, nullptr);
		if (!window)
		{
			glfwTerminate();
			throw std::runtime_error("Failed to create GLFW window!");
		}
	}

	GLFWWindow::~GLFWWindow()
	{
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void GLFWWindow::Initialize(MauRen::Renderer* pRenderer)
	{
		glfwSetWindowUserPointer(window, pRenderer);
		glfwSetFramebufferSizeCallback(window, FramebufferResizeCallback);
	}
}
