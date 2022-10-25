#ifndef _CORE_WINDOW_H_
#define _CORE_WINDOW_H_

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
namespace VulkanPathfinding{
    class Window
    {
    public:
        Window(const int& width, const int& height, const std::string&& title);
        ~Window();
        void ProcessEvents();
        //void GetSurface();
        
    private:
        
        int m_width;
        int m_heigth;
        std::string m_title;
        GLFWwindow* m_windowHandle;

        
    };    
}
#endif