#ifndef _CORE_APPLICATION_H_
#define _CORE_APPLICATION_H_

//STL
#include <iostream>


//Own
#include "../window/Window.h"

#include "../time/Timer.h"
#include "../time/DeltaTime.h"
#include "../time/FPSCounter.h"

#include "Logger.h"
namespace VulkanPathfinding{

class Application
{
public:
    Application(bool enableValidation);
    ~Application();
    void Run();
    std::shared_ptr<Window> GetWindow(){return m_window;}
private:
    static std::shared_ptr<Window> m_window;
    
    void Initialize();
    void Shutdown();

    void InitVulkan();
    
    VkResult CreateVulkanInstance(bool enableValidation);

    bool m_enableValidation;
    bool m_isRunning;

    FPSCounter m_fpsCounter;
    DeltaTime m_deltaTime;

};
}
#endif