#include "VulkanDebug.h"

namespace Pathfinding
{

    VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebug::VulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData)
    {
        std::string temp;
        size_t index;
        switch (messageSeverity)
        {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            APP_TRACE("Vulkan {0}", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:

            if(pCallbackData->pMessageIdName ==std::string("UNASSIGNED-DEBUG-PRINTF")){
            temp = std::string(pCallbackData->pMessage);
            index= temp.find_last_of('|');
            index+=1;
            temp = temp.substr(index, temp.length());           
            APP_INFO("{0}", temp);
             }else{
                 APP_INFO("Vulkan {0}", pCallbackData->pMessage);
             }
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            APP_WARN("Vulkan {0}", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            APP_ERROR("Vulkan {0}", pCallbackData->pMessage);
            break;
        default:
            break;
        }
        (void)messageType;
        (void)pUserData;

        return VK_FALSE;
    }

    VkDebugUtilsMessengerCreateInfoEXT VulkanDebug::s_debugCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .pNext = nullptr,
        .flags = 0,
        //to turn off shader debuging change serverity on off remove VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT flag 
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = VulkanDebugCallback,
        .pUserData = nullptr};

    VkDebugUtilsMessengerEXT VulkanDebug::s_debugMessenger = nullptr;
    
    void VulkanDebug::Initialize(VkInstance instance)
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        func(instance, &s_debugCreateInfo, nullptr, &s_debugMessenger);
    }

    void VulkanDebug::Shutdown(VkInstance instance)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        func(instance, s_debugMessenger, nullptr);
    }
} 


