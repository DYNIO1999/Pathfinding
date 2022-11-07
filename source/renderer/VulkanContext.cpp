#include "VulkanContext.h"
#include "../core/Application.h"

namespace Pathfinding{

    VkInstance VulkanContext::m_vulkanInstanceHandle = VK_NULL_HANDLE;

    VulkanContext::VulkanContext(){
        Initialize();
    }
    VulkanContext::~VulkanContext(){
        Shutdown();
    }

    void VulkanContext::Initialize()
    {

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Pathfinding";
        appInfo.pEngineName = "Pathfinding";
        appInfo.apiVersion = VK_API_VERSION_1_3;

        std::vector<const char *> enabledLayers = GetRequiredLayers();
        std::vector<const char *> enabledExtensions = GetRequiredExtensions();

        VkInstanceCreateInfo instanceCreateInfo = {};
        instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.ppEnabledLayerNames = enabledLayers.data();
        instanceCreateInfo.enabledLayerCount = (uint32_t)enabledLayers.size();
        instanceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
        instanceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
        instanceCreateInfo.pApplicationInfo = &appInfo;
        instanceCreateInfo.pNext = VulkanDebug::GetDebugCreateInfo();
        
        
        VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &m_vulkanInstanceHandle);
        if (result) {
            CHECK_ERROR(APP_ERROR_VALUE, APP_ERROR("Vulkan Instance creation failed"));
        }
        
        if(Application::IsValidationEnabled()){
            VulkanDebug::Initialize(m_vulkanInstanceHandle);
        }


    }

    void VulkanContext::Shutdown(){

        if(Application::IsValidationEnabled())
            VulkanDebug::Shutdown(m_vulkanInstanceHandle);

        vkDestroyInstance(m_vulkanInstanceHandle, nullptr);
    }   
    std::vector<const char *> VulkanContext::GetRequiredLayers()
    {
        std::vector<const char *> layers;
        if (Application::IsValidationEnabled())
        {
            layers.push_back("VK_LAYER_KHRONOS_validation");
        }

        return layers;
    }
    std::vector<const char *> VulkanContext::GetRequiredExtensions()
    {
        uint32_t count;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&count);

        std::vector<const char *> extensions(glfwExtensions, glfwExtensions + count);
        
        if(Application::IsValidationEnabled()){
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
            extensions.emplace_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        }
        return extensions;
    }
}