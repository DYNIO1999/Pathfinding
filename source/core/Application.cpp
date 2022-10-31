#include "Application.h"
#include "../input/Input.h"
namespace VulkanPathfinding{

    std::shared_ptr<Window> Application::m_window;

    Application::Application(bool enableValidation)
    :m_enableValidation(enableValidation)
    {
    
    }
    Application::~Application(){

    }

    void Application::Run(){
        //Timer timer(true);
        Initialize();



        //Init Vulkan
        while (m_window->IsOpen())
        {

            //Delta Time
            m_deltaTime.Update(static_cast<float>(glfwGetTime()));
            //APP_TRACE("Delta Time: {}", m_deltaTime.AsMiliSeconds());
            //Delta Time
            
            //FRAME START
            //dooo update
            //doo render
          
            //auto [x,y] = Input::MousePosition();
            //APP_INFO("MOUSE_POSITION  X:{} Y:{}",x,y);

            //FRAME END
            m_window->ProcessEvents(); //process events/inputs                        
            
            m_fpsCounter.Update();
            
            //if(m_fpsCounter.GetFPS()>0)
            //    APP_TRACE("FPS {}", m_fpsCounter.GetFPS());
            
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

