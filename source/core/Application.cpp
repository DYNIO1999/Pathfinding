#include "Application.h"
#include "Timer.h"
namespace VulkanPathfinding{

    std::shared_ptr<Window> Application::m_window;

    Application::Application(bool enableValidation)
    :m_enableValidation(enableValidation),
    m_isRunning(true)
    {
    
    }
    Application::~Application(){

    }

    void Application::Run(){
        Timer timer;
        Initialize();

        //Init Vulkan
        while (m_isRunning)
        {
            m_deltaTime.CalculateDeltaTime(static_cast<float>(glfwGetTime()));
            //dooo update
            //doo render
            m_window->ProcessEvents(); //process events                        
            APP_TRACE("Delta Time: {}", m_deltaTime.AsMiliSeconds());
            //APP_TRACE("FPS Counter: {}", m_fpsCounter.Fps());
            m_deltaTime.Update();
            
        }
        Shutdown();

    } 


    void Application::Initialize(){
        Timer timer(GET_NAME(Initialize()));
        Logger::Init();

        m_window = std::make_shared<Window>(1600,900,std::move("VulkanPathfinding"));

        InitVulkan();


    }
    void Application::Shutdown(){

    }
    
    void Application::InitVulkan(){
        
    }
    
}

