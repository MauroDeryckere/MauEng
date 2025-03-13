
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <iostream>


int main()
{
    glm::vec3 position(1.0f, 2.0f, 3.0f);
    std::cout << "GLM vec3: ("
        << position.x << ", "
        << position.y << ", "
        << position.z << ")\n";

    // Test GLFW: initialize and terminate
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW!\n";
        return -1;
    }
    std::cout << "GLFW initialized successfully.\n";
    glfwTerminate();


	while (true)
	{
		
	}
	return 0;
}