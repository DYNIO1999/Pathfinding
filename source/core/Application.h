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
#include "../renderer/VulkanAllocator.h"
#include "../renderer/VulkanBuffers.h"

#include "../objects/Object.h"



#include "Logger.h"


namespace Pathfinding{

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


    void InitObjects();
private:
    std::unique_ptr<VulkanContext> m_context;
    static std::shared_ptr<Window> m_window;
    std::unique_ptr<VulkanDevice> m_device;
    std::unique_ptr<VulkanAllocator> m_allocator;
    std::unique_ptr<VulkanSwapChain> m_swapchain;
    std::unique_ptr<VulkanPipeline> m_defaultPipline;
    
    
    
    void Initialize();
    void Shutdown();

    static bool m_enableValidation;
    
    FPSCounter m_fpsCounter;
    DeltaTime m_deltaTime;
    PipelineSpecification m_defaultPipelineSpec{};

    std::vector<VkCommandBuffer> commandBuffers;
    
    //TESTING DATA BELOW
    std::unique_ptr<VulkanIndexBuffer> m_indexBuffer;
    std::unique_ptr<VulkanVertexBuffer> m_vertexBuffer;
    std::unique_ptr<VulkanVertexBuffer> m_vertexBuffer2;

    std::vector<uint32_t> indices = {0, 1, 2, 2, 3, 0};

    std::vector<Vertex> vertices;
    std::vector<Vertex> vertices_2;
    void CreateVertexBuffer();

    glm::vec3 camPos = {0.f, 0.f, -2.f};

    glm::mat4 view;
    // camera projection
    glm::mat4 projection = glm::perspective(glm::radians(70.f), 1600.f / 900.f, 0.1f, 200.0f);
    // model rotation
    glm::mat4 model = glm::mat4(1);


    std::vector<Mesh> m_objects;
};
}
#endif