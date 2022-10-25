#ifndef _CORE_WINDOW_H_
#define _CORE_WINDOW_H_

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
namespace VulkanPathfinding{
    class Window
    {
    public:
        Window(int width, int height, std::string title);
        ~Window();
        //void GetSurface();
    private:
        int m_width;
        int m_heigth;
        std::string title;
        GLFWwindow* m_windowHandle;
    };    
}
#endif