#ifndef _RENDERER_VULKAN_CONTEXT_H_
#define _RENDERER_VULKAN_CONTEXT_H_

#include <vector>

#include "VulkanDebug.h" 

namespace VulkanPathfinding{

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

    VkInstance InstanceHandle() { return m_vulkanInstanceHandle; }
    void Initialize();
    void Shutdown();

private:
    //Private Methods Vulkan
    std::vector<const char*> GetRequiredLayers();
    std::vector<const char*> GetRequiredExtensions();

    VkInstance m_vulkanInstanceHandle;
};
}
#endif