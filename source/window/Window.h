#ifndef _CORE_WINDOW_H_
#define _CORE_WINDOW_H_

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
namespace Pathfinding{
    class Window
    {
    public:
        Window(const int& width, const int& height, const std::string&& title);
        ~Window();
        void ProcessEvents();
        bool IsOpen() const { return !glfwWindowShouldClose(m_windowHandle);}
        //void GetSurface();
        GLFWwindow* GetWindowHandle() const {return m_windowHandle;}

        static void WindowCloseCallback(GLFWwindow *window);
        static void WindowSizeCallback(GLFWwindow *window, int width, int height);
        void CreateSurface(VkSurfaceKHR* surface);
        std::pair<int, int> WindowSize() { return {m_width, m_heigth}; }

    private:
    private:
        
        int m_width;
        int m_heigth;
        std::string m_title;
        GLFWwindow* m_windowHandle;

        
    };    
}
#endif