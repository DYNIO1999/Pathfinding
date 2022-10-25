#ifndef _CORE_APPLICATION_H_
#define _CORE_APPLICATION_H_

//STL
#include <iostream>


//Own
#include "../window/Window.h"


namespace VulkanPathfinding{
class Application
{
public:
    Application(bool enableValidation);
    ~Application();
    void Run();
private:
    void Initialize();
    void Shutdown();




    void InitVulkan();
    
    VkResult CreateVulkanInstance(bool enableValidation);

    bool m_enableValidation;
    bool m_isRunning;


    int m_frameRate;
};
}
#endif