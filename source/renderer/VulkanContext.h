#ifndef _RENDERER_VULKAN_CONTEXT_H_
#define _RENDERER_VULKAN_CONTEXT_H_

#include <vector>
#include <optional>
#include <set>
#include <limits>

#include "VulkanDebug.h" 

namespace VulkanPathfinding{

const int MAX_FRAMES_IN_FLIGHT = 2;    

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily; 
    std::optional<uint32_t> presentFamily;
    bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

//SingleInstance [SINGLTEON]
class VulkanContext
{
public:
    static VulkanContext &Get()
    {
        static VulkanContext instance; 
                                       
        return instance;
    }
    VulkanContext()=default;
    VulkanContext(const VulkanContext&) = delete;
    void operator=(const VulkanContext&) = delete;

    void Initialize();
    void Shutdown();

    VkInstance GetInstanceHandle(){return m_vulkanInstanceHandle;}
    VkSurfaceKHR* GetSurfaceHandle(){return &m_surfaceHandle;}

    void SelectPhysicalDevice();
    void CreateLogicalDevice();
    void InitSwapChain();
    void CreateImageViews();
    void CreateCommandPools();
    void CreateCommandBuffers();
    void CreateSynchronizationObjects();
    void SetupRenderPass();
    void CreateFrameBuffers();
    

private:


    QueueFamilyIndices FindQueueFamily();
    std::vector<const char*> GetRequiredLayers();
    std::vector<const char*> GetRequiredExtensions();
    
    
    VkInstance m_vulkanInstanceHandle;


    //Physical device
    VkPhysicalDevice m_vulkanPhysicalDeviceHandle;
    VkPhysicalDeviceProperties m_deviceProperties;
    VkPhysicalDeviceFeatures m_deviceFeatures;
    VkPhysicalDeviceMemoryProperties m_deviceMemoryProperties;

    QueueFamilyIndices m_queueFamilyIndices;

    std::vector<VkQueueFamilyProperties> m_queueFamilyProperties;

    std::vector<const char *> m_deviceExtensions;

    //Logical device
    VkDevice m_vulkanLogicalDeviceHandle;
    //Surface
    VkSurfaceKHR m_surfaceHandle;
    //Queues 
    VkQueue m_vulkanGraphicsQueueHandle;
    VkQueue m_vulkanPresentationQueueHandle;



    //SwapChain
    VkSurfaceCapabilitiesKHR m_vulkanSwapchainCapabilities;   
    std::vector<VkSurfaceFormatKHR> m_vulkanSwapchainFormats;
    std::vector<VkPresentModeKHR> m_vulkanSwapchainPresentModes;

    VkFormat m_vulkanCurrentSwapChainImageFormat;
    VkExtent2D m_vulkanCurrentSwapChainExtent;

    uint32_t m_vulkanSwapChainImagesCount;
    std::vector<VkImage> m_vulkanSwapChainImages;
    std::vector<VkImageView> m_vulkanSwapChainImageViews;

    VkSwapchainKHR m_vulkanSwapChainHandle;
    //
    //Command Pool
    VkCommandPool m_vulkanCommandPoolHandle;

    //Command Buffer
    std::vector<VkCommandBuffer> m_vulkanCommandBuffers;

    //WaitFences
    std::vector<VkFence> m_vulkanWaitFences;


    //RenderPass
    VkRenderPass m_vulkanRenderPassHandle;
    //Framebuffers
    std::vector<VkFramebuffer> m_vulkanFrameBuffers;
};
}
#endif