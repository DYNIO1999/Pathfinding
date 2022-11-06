#include "Window.h"
#include "../core/Logger.h"
#include "../input/Input.h"
#include "../renderer/VulkanContext.h"
namespace VulkanPathfinding
{

    static void glfwErrorCallback(int code, const char *error)
    {
      APP_ERROR("GLFW_ERROR: {0} ({1})", error, code);
    }
    Window::Window(const int& width, const int& height, const std::string&& title):
    m_width(width), m_heigth(height),m_title(title)
    {
        glfwSetErrorCallback(glfwErrorCallback);
      
        int check=glfwInit(); 
        if (check==0)
        {
            CHECK_ERROR(check, APP_ERROR("GLFW Initialization failed"));
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        m_windowHandle = glfwCreateWindow(m_width, m_heigth, m_title.c_str(),nullptr,nullptr);
        
        if(!m_windowHandle){
            CHECK_ERROR(APP_ERROR_VALUE, APP_ERROR("GLFW Window creation failed"));
        }

        glfwSetWindowSizeCallback(m_windowHandle,WindowSizeCallback);
        glfwSetWindowCloseCallback(m_windowHandle, WindowCloseCallback);

    }
    void Window::ProcessEvents(){
        glfwPollEvents();
    }
    Window::~Window()
    {
        glfwDestroyWindow(m_windowHandle);
        glfwTerminate();
    }
    void Window::WindowCloseCallback(GLFWwindow *window){
        APP_INFO("WINDOW CLOSED");
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
     void Window::WindowSizeCallback(GLFWwindow *window, int width, int height){
        (void)window;
        APP_INFO("RESIZED WIDTH:{} HEIGHT:{}", width, height);
    }

    void Window::CreateSurface(VkSurfaceKHR* surface)
    {
        if (glfwCreateWindowSurface(VulkanContext::InstanceHandle(), m_windowHandle, nullptr, surface) != VK_SUCCESS)
        {
            CHECK_ERROR(APP_ERROR_VALUE, APP_ERROR("Failed to create window surface!"));
        }
    }
}