#include <stdexcept>

#include "GlfwWindow.h"

#include "GLFW/glfw3.h"

namespace MauEng
{
	GLFWWindow::GLFWWindow()
	{
		if (not glfwInit() == GLFW_TRUE)
		{
			throw std::runtime_error("Failed to initialize GLFW!");
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

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
}
