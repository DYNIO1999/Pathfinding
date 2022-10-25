#ifndef _CORE_APPLICATION_H_
#define _CORE_APPLICATION_H_

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

//STL
#include <iostream>



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
};
}
#endif