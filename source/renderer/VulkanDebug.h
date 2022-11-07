#ifndef _RENDERER_VULKAN_DEBUG_H_
#define _RENDERER_VULKAN_DEBUG_H_

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "../core/Logger.h"

namespace Pathfinding{
    
    
    class VulkanDebug
    {
    public:
        static void Initialize(VkInstance instance);
        static void Shutdown(VkInstance instance);
        static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData);
        
        static VkDebugUtilsMessengerCreateInfoEXT* GetDebugCreateInfo(){
            return &s_debugCreateInfo;
        }
    private:
        static VkDebugUtilsMessengerEXT s_debugMessenger;
        static VkDebugUtilsMessengerCreateInfoEXT s_debugCreateInfo;
    };
}
#endif