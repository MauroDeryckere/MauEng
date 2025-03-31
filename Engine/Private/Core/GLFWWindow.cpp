#include <stdexcept>

#include "GlfwWindow.h"

#include "ServiceLocator.h"
#include "GLFW/glfw3.h"

namespace MauEng
{
	void GLFWWindow::FramebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		auto const ptr{ glfwGetWindowUserPointer(window) };
		GLFWWindow* winClassPtr{ static_cast<GLFWWindow*>(ptr) };

		winClassPtr->width = width;
		winClassPtr->height = height;

		ServiceLocator::GetRenderer().ResizeWindow();

		// TODO reflact width / heigh change in the camera
	}

	GLFWWindow::GLFWWindow()
	{
		if (not (glfwInit() == GLFW_TRUE))
		{
			throw std::runtime_error("Failed to initialize GLFW!");
		}
		glfwSwapInterval(0);

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

	void GLFWWindow::Initialize()
	{
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, FramebufferResizeCallback);
	}
}
