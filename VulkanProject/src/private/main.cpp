#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cassert>

GLFWwindow* g_Window{ nullptr };

void InitWindow()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	uint32_t constexpr WIDTH{ 800 };
	uint32_t constexpr HEIGHT{ 600 };

	g_Window = glfwCreateWindow(WIDTH, HEIGHT, "Mauro Deryckere - Vulkan Project", nullptr, nullptr);

}

void InitVulkan()
{

}

void MainLoop()
{
	assert(g_Window);
	while (!glfwWindowShouldClose(g_Window))
	{
		glfwPollEvents();
	}
}

void Cleanup()
{
	glfwDestroyWindow(g_Window);
	glfwTerminate();
}

void Run()
{
    InitWindow();
	InitVulkan();
    MainLoop();
    Cleanup();
}

int main()
{
	try 
	{
		Run();
	}
	catch (const std::exception& e) 
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
