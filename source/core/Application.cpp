#include "Application.h"

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
        Timer timer(true);
        Initialize();



        //Init Vulkan
        while (m_isRunning)
        {

            //Delta Time
            m_deltaTime.Update(static_cast<float>(glfwGetTime()));
            APP_TRACE("Delta Time: {}", m_deltaTime.AsMiliSeconds());
            //Delta Time
            
            //FRAME START
            //dooo update
            //doo render
            for (size_t i = 0; i < 10000000; i++)
            {
                
            }
            
            //FRAME END
            m_window->ProcessEvents(); //process events/inputs                        
            
            m_fpsCounter.Update();

            if(m_fpsCounter.GetFPS()>0)
                APP_TRACE("FPS {}", m_fpsCounter.GetFPS());
            
        }
        Shutdown();

    } 


    void Application::Initialize(){
        Timer timer(true,GET_NAME(Initialize()));
        Logger::Init();

        m_window = std::make_shared<Window>(1600,900,std::move("VulkanPathfinding"));

        InitVulkan();
    }
    void Application::Shutdown(){

    }
    
    void Application::InitVulkan(){
        
    }
    
}

