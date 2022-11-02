#ifndef _RENDERER_VULKAN_CONTEXT_H_
#define _RENDERER_VULKAN_CONTEXT_H_

#include <vector>
#include <optional>
#include <set>

#include "VulkanDebug.h" 

namespace VulkanPathfinding{

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


private:


    QueueFamilyIndices FindQueueFamily();

    //Private Methods Vulkan
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

    //Logical device
    VkDevice m_vulkanlogicalDeviceHandle;
    //Surface

    VkSurfaceKHR m_surfaceHandle;
};
}
#endif