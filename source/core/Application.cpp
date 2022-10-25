#include "Application.h"
#include "Timer.h"
namespace VulkanPathfinding{

    Application::Application(bool enableValidation)
    :m_enableValidation(enableValidation),
    m_isRunning(true)
    {
    
    }
    Application::~Application(){

    }

    void Application::Run(){
        
        Initialize();

        
        //Init Vulkan
        while (m_isRunning)
        {
            
            
        }
        Shutdown();

        
    }


    void Application::Initialize(){
        int test;
        Timer timer(GET_NAME(Initialize()));
        Logger::Init();
        InitVulkan();


    }
    void Application::Shutdown(){

    }
    
    void Application::InitVulkan(){
        
    }
    
}

