#ifndef _RENDERER_VULKAN_DEVICE_H_
#define _RENDERER_VULKAN_DEVICE_H_

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


#include <memory>
#include <vector>
#include <set>
#include <optional>

namespace VulkanPathfinding{

struct SwapChainSupportInfo
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamilyIndicesInfo
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }  
};

class VulkanDevice
{
public:
    VulkanDevice();
    ~VulkanDevice();

    SwapChainSupportInfo GetSwapChainSupport() { return QuerySwapChainSupport(m_physicalDeviceHandle); }

    QueueFamilyIndicesInfo FindPhysicalQueueFamilies() { return FindQueueFamilies(m_physicalDeviceHandle); }

    VkPhysicalDeviceProperties m_physicalDeviceProperties;

    VkPhysicalDeviceFeatures m_physicalDeviceFeatures;
    VkPhysicalDeviceMemoryProperties m_physicalDeviceMemoryProperties;


    VkQueue GraphicsQueueHandle() { return m_graphicsQueueHandle; }
    VkSurfaceKHR SurfaceHandle() { return m_surfaceHandle; }

    VkCommandPool CommandPoolHandle() { return m_commandPoolHandle; }
    VkQueue PresentQueueHandle() { return m_presentQueueHandle; }
    VkDevice LogicalDeviceHandle() { return m_logicalDeviceHandle; }

private:

    void Initialize();
    void Shutdown();
    
    void CreateSurface();
    void SelectPhysicalDevice();
    void CreateLogicalDevice();
    void CreateCommandPool();



    QueueFamilyIndicesInfo FindQueueFamilies(VkPhysicalDevice device);
    SwapChainSupportInfo QuerySwapChainSupport(VkPhysicalDevice device);

    VkPhysicalDevice m_physicalDeviceHandle = VK_NULL_HANDLE;
    VkCommandPool m_commandPoolHandle;

    VkDevice m_logicalDeviceHandle;
    VkSurfaceKHR m_surfaceHandle;
    VkQueue m_graphicsQueueHandle;
    VkQueue m_presentQueueHandle;
    std::vector<const char *> m_deviceExtensions;
};


}
#endif
