#ifndef _RENDERER_VULKAN_CONTEXT_H_
#define _RENDERER_VULKAN_CONTEXT_H_

#include <vector>

#include "VulkanDebug.h" 

namespace VulkanPathfinding{

class VulkanContext
{
public:

    VulkanContext();
    ~VulkanContext();
    VulkanContext(const VulkanContext&) = delete;
    void operator=(const VulkanContext&) = delete;

    static VkInstance InstanceHandle() { return m_vulkanInstanceHandle;}
private:
    void Initialize();
    void Shutdown();
    std::vector<const char*> GetRequiredLayers();
    std::vector<const char*> GetRequiredExtensions();

    static VkInstance m_vulkanInstanceHandle;
};
}
#endif