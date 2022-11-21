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
#include "Logger.h"

#include "camera/Camera.h"
#include "../renderer/VulkanBuffer.h"
#include "../pathfinding/Pathfinding.h"

namespace Pathfinding{

struct VulkanBufferTest{
    VkBuffer bufferHandle = VK_NULL_HANDLE;
    VmaAllocation allocationHandle;
    float size;
    void* data; 
};

struct ShaderStorageBufferObject{
    VulkanBufferTest buffer;
    std::vector<int> data;
    VkDescriptorSet descriptor;
};

struct CameraUBO{
    VulkanBufferTest buffer;
    struct Values
    {
        glm::mat4 view;
        glm::mat4 proj;
    } values;
};

struct GlobalCameraData{
    VkDescriptorSet descriptors[2];
    std::unique_ptr<VulkanBuffer> cameraUBOs[2]; //cause 2 frames in flight
};

struct ModelData{
    glm::mat4 transform; //To uniform buffer

    bool isPassable{true};
    int index;
    glm::vec3 position{0.0f,0.0f,0.0f};
    glm::vec4 color{0.47, 0.47, 0.48, 1.0};
    VkDescriptorSet descriptors[2];
    std::unique_ptr<VulkanBuffer> modelUBOs[2]; // cause 2 frames in flight
};

struct AgentGraphicData{
    glm::mat4 transform; //To uniform buffer

    glm::vec3 position{0.0f, 0.0f, 0.0f};
    glm::vec4 color;

    VkDescriptorSet descriptors[2];
    std::unique_ptr<VulkanBuffer> modelUBOs[2];
    std::vector<int> path;
};


struct ObstacleGraphicData{
    glm::mat4 transform; //To uniform buffer

    glm::vec3 position{0.0f, 0.0f, 0.0f};
    glm::vec4 color{0.0, 0.0, 0.0, 1.0};

    VkDescriptorSet descriptors[2];
    std::unique_ptr<VulkanBuffer> modelUBOs[2];
};

class Application
{
public:
    Application(bool enableValidation);
    ~Application();
    void Run();

    static std::shared_ptr<Window> GetWindow(){return m_window;}
    static bool IsValidationEnabled() { return m_enableValidation; }
    
    void Update();
    void Draw();
    
    void CreateCommandBuffers();
    void UpdateUniformBuffer(uint32_t currentFrame);
    void CreateUniformBuffer();

    void InitObjects();

private:
    std::unique_ptr<VulkanContext> m_context;
    static std::shared_ptr<Window> m_window;
    std::unique_ptr<VulkanDevice> m_device;
    std::unique_ptr<VulkanAllocator> m_allocator;
    std::unique_ptr<VulkanSwapChain> m_swapchain;
    std::unique_ptr<VulkanPipeline> m_defaultPipline;

    
    GlobalCameraData m_cameraData{};
    std::vector<ModelData> m_gridData;
    
    
    
    void Initialize();
    void Shutdown();

    static bool m_enableValidation;
    
    FPSCounter m_fpsCounter;
    DeltaTime m_deltaTime;
    PipelineSpecification m_defaultPipelineSpec{};

    std::vector<VkCommandBuffer> commandBuffers;

    void CreateDescriptorPool();
    void CreateDescriptorSets();
    

    glm::mat4 projection = glm::perspective(glm::radians(70.f), 1600.f / 900.f, 0.1f, 200.0f);
    glm::mat4 model = glm::mat4(1);

    VkDescriptorPool m_descriptorPool;
    
    std::unique_ptr<VulkanBuffer> m_vertexBuffer;
    std::unique_ptr<VulkanBuffer> m_indexBuffer;

    std::vector<Vertex> m_vertices;
    std::vector<u_int32_t> m_indices{0, 1, 2, 
                                     2, 1, 3,
                                     4, 0, 6, 
                                     6, 0, 2,
                                     7, 5, 6, 
                                     6, 5, 4,
                                     3, 1, 7, 
                                     7, 1, 5,
                                     4, 5, 0, 
                                     0, 5, 1,
                                     3, 7, 2, 
                                     2, 7, 6};

    std::unique_ptr<Camera> m_camera;

    
    //compute Testing

    void CreateComputePipelineLayout();
    void CreateComputePipeline(const std::string& shaderPath);
    void CreateComputeStorageBuffers();
    void CreateComputeDescriptorPool();

    void CreateComputeCommandPool();
    void BuildComputeCommands();
    void CalculateCompute();
    VkCommandBuffer m_computeCommandBuffer;
    std::vector<VkDescriptorSetLayout> m_computeSetLayouts;
    VkPipelineLayout m_pipelineLayoutCompute;
    VkPipeline m_computePipeline{VK_NULL_HANDLE};
    VkDescriptorPool m_descriptorPoolCompute;
    ShaderStorageBufferObject m_ssbObjects[2];

   
    //Testing

    std::vector<int> m_obstaclesIndexes;
    std::vector<ObstacleGraphicData> m_obstacles;
    std::vector<AgentGraphicData> m_agents;

    //void TestingAbstraction();
    void InitGrids();


    GridData grid;
    void ResolvePath();
    
    bool started{false};
    glm::vec3 currentPos;
    glm::vec3 neighbourPos;
    glm::vec3 differnecePos;
    float length;
    glm::vec3 direction;


    bool pressed{false};
};
}
#endif