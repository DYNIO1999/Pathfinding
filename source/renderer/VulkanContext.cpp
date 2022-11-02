#include "VulkanContext.h"
#include "../core/Application.h"

namespace VulkanPathfinding{

    void VulkanContext::Initialize()
    {

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "VulkanPathfinding";
        appInfo.pEngineName = "VulkanPathfinding";
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

        //setup debug
        if(Application::IsValidationEnabled()){
            VulkanDebug::Initialize(m_vulkanInstanceHandle);
        }
    }

    void VulkanContext::Shutdown(){

        vkDestroySurfaceKHR(m_vulkanInstanceHandle,m_surfaceHandle,nullptr);

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

    void VulkanContext::SelectPhysicalDevice(){

        // chooose physical device

        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(m_vulkanInstanceHandle, &deviceCount, nullptr);
        if (deviceCount == 0)
        {
            CHECK_ERROR(APP_ERROR_VALUE, APP_ERROR("Failed to find GPUs with Vulkan support!"));
        }
        std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
        vkEnumeratePhysicalDevices(m_vulkanInstanceHandle, &deviceCount, physicalDevices.data());

        VkPhysicalDevice selectedPhysicalDevice = nullptr;
        for (VkPhysicalDevice physicalDevice : physicalDevices)
        {
            vkGetPhysicalDeviceProperties(physicalDevice, &m_deviceProperties);
            if (m_deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                selectedPhysicalDevice = physicalDevice;
                break;
            }
        }
        if (!selectedPhysicalDevice)
        {
            APP_WARN("Could not find discrete GPU.");
            selectedPhysicalDevice = physicalDevices.back();
        }

        if (!selectedPhysicalDevice)
        {
            CHECK_ERROR(APP_ERROR_VALUE, APP_ERROR("Could not find any suitable GPU!"));
        }
        m_vulkanPhysicalDeviceHandle = selectedPhysicalDevice;

        vkGetPhysicalDeviceProperties(m_vulkanPhysicalDeviceHandle, &m_deviceProperties);
        APP_INFO("NAME {}", m_deviceProperties.deviceName);
        vkGetPhysicalDeviceFeatures(m_vulkanPhysicalDeviceHandle, &m_deviceFeatures);
        vkGetPhysicalDeviceMemoryProperties(m_vulkanPhysicalDeviceHandle, &m_deviceMemoryProperties);

        // Queue family properties, used for setting up requested queues upon device creation
        uint32_t queueFamilyCount;
        vkGetPhysicalDeviceQueueFamilyProperties(m_vulkanPhysicalDeviceHandle, &queueFamilyCount, nullptr);
        assert(queueFamilyCount > 0);
        
        
        m_queueFamilyProperties.resize(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(m_vulkanPhysicalDeviceHandle, &queueFamilyCount, m_queueFamilyProperties.data());

        //
    }

    void VulkanContext::CreateLogicalDevice(){
        
        m_queueFamilyIndices = FindQueueFamily();

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {m_queueFamilyIndices.graphicsFamily.value(), m_queueFamilyIndices.presentFamily.value()};

        float queuePriority = 1.0f; 
        for (uint32_t queueFamily : uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1; 
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        
    }
    QueueFamilyIndices VulkanContext::FindQueueFamily(){
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(m_vulkanPhysicalDeviceHandle, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(m_vulkanPhysicalDeviceHandle, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto &queueFamily : queueFamilies) 
        {
            if (((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0 )&& ((queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0))
            {
                indices.graphicsFamily = i; 
            }
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(m_vulkanPhysicalDeviceHandle, i, m_surfaceHandle, &presentSupport);
            if (presentSupport)
            {
                indices.presentFamily = i; 
            }

            if (indices.isComplete()) 
            {
                break;
            }
            i++;
        }
        return indices;
    }


}