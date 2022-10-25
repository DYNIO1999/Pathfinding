#include "Window.h"
#include "../core/Logger.h"

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
            ERROR(check, APP_ERROR("GLFW Initialization failed"));
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        m_windowHandle = glfwCreateWindow(m_width, m_heigth, m_title.c_str(),nullptr,nullptr);
        
        if(!m_windowHandle){
          ERROR(0, APP_ERROR("GLFW Window creation failed"));
        }


//          glfwSwapInterval(1);
    }
    void Window::ProcessEvents(){
        glfwPollEvents();
    }
    Window::~Window()
    {
        glfwDestroyWindow(m_windowHandle);
        glfwTerminate();
    }
}