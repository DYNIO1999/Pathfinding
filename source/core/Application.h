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

#include "../objects/Object.h"
#include "../renderer/VulkanInitializers.h"
#include "Logger.h"


namespace Pathfinding{


struct VulkanBuffer{
    VkBuffer bufferHandle = VK_NULL_HANDLE;
    VmaAllocation allocationHandle;
    float size;
    void* data; 
};

struct UniformBuffer{
    VkBuffer bufferHandle = VK_NULL_HANDLE;
    VmaAllocation allocationHandle;
    void *data;
    struct Values{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
    }values;
};

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
    void UpdateUniformBuffer(uint32_t currentFrame);
    void CreateUniformBuffer();

    void InitObjects();

    std::vector<glm::mat4> transforms;

private:
    std::unique_ptr<VulkanContext> m_context;
    static std::shared_ptr<Window> m_window;
    std::unique_ptr<VulkanDevice> m_device;
    std::unique_ptr<VulkanAllocator> m_allocator;
    std::unique_ptr<VulkanSwapChain> m_swapchain;
    std::unique_ptr<VulkanPipeline> m_defaultPipline;
    std::vector<VkDescriptorSet> m_descriptors;
    std::vector<UniformBuffer> m_uniformBuffers;
    
    
    
    void Initialize();
    void Shutdown();

    static bool m_enableValidation;
    
    FPSCounter m_fpsCounter;
    DeltaTime m_deltaTime;
    PipelineSpecification m_defaultPipelineSpec{};

    std::vector<VkCommandBuffer> commandBuffers;

   

    

    void CreateVertexBuffer();
    void CreateIndexBuffer();
    void CreateDescriptorPool();
    void CreateDescriptorSets();
    glm::vec3 camPos = {0.f, 0.f, -2.f};
    glm::mat4 view = glm::translate(glm::mat4(1.f), camPos);
    ;
    glm::mat4 projection = glm::perspective(glm::radians(70.f), 1600.f / 900.f, 0.1f, 200.0f);
    glm::mat4 model = glm::mat4(1);

    VkDescriptorPool m_descriptorPool;
    VulkanBuffer m_vertexBuffer{};
    VulkanBuffer m_indexBuffer{};
    std::vector<Vertex> m_vertices;
    std::vector<u_int32_t> m_indices{0, 1, 2, 2, 3, 0};
};
}
#endif