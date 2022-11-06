#ifndef _CORE_APPLICATION_H_
#define _CORE_APPLICATION_H_

//STL
#include <iostream>
#include <filesystem>

//Own
#include "../window/Window.h"

#include "../time/Timer.h"
#include "../time/DeltaTime.h"
#include "../time/FPSCounter.h"

#include "../renderer/VulkanContext.h"
#include "../renderer/VulkanSwapChain.h"
#include "../renderer/VulkanPipeline.h"



#include "Logger.h"


namespace VulkanPathfinding{

class Application
{
public:
    Application(bool enableValidation);
    ~Application();
    void Run();

    static std::shared_ptr<Window> GetWindow(){return m_window;}
    static bool IsValidationEnabled() { return m_enableValidation; }

    void Draw();
    void CreateCommandBuffers();
private:
    std::unique_ptr<VulkanContext> m_context;
    static std::shared_ptr<Window> m_window;
    std::unique_ptr<VulkanDevice> m_device;
    std::unique_ptr<VulkanSwapChain> m_swapchain;

    std::unique_ptr<VulkanPipeline> m_defaultPipline;
    void Initialize();
    void Shutdown();

    static bool m_enableValidation;
    
    FPSCounter m_fpsCounter;
    DeltaTime m_deltaTime;
    PipelineSpecification m_defaultPipelineSpec{};

    std::vector<VkCommandBuffer> commandBuffers;
};
}
#endif