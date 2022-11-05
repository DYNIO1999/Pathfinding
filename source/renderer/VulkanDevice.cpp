#include "VulkanDevice.h"
#include "VulkanContext.h"
#include "../core/Application.h"
namespace VulkanPathfinding
{
    VulkanDevice::VulkanDevice() 
    { 
    }
    VulkanDevice::~VulkanDevice()
    {

    }

    void VulkanDevice::Initialize()
    {
        CreateSurface();
        SelectPhysicalDevice();
        CreateLogicalDevice();
        CreateCommandPool();
    }
    void VulkanDevice::Shutdown()
    {
        vkDestroyCommandPool(m_logicalDeviceHandle, m_commandPoolHandle, nullptr);
        vkDestroyDevice(m_logicalDeviceHandle, nullptr);
        vkDestroySurfaceKHR(VulkanContext::Get().InstanceHandle(), m_surfaceHandle, nullptr);
    }
    void VulkanDevice::CreateSurface()
    {
        Application::GetWindow()->CreateSurface(&m_surfaceHandle);
    }
    void VulkanDevice::SelectPhysicalDevice()
    {
        auto context = VulkanContext::Get().InstanceHandle();

        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(context, &deviceCount, nullptr);
        if (deviceCount == 0)
        {
            CHECK_ERROR(APP_ERROR_VALUE, APP_ERROR("Failed to find GPUs with Vulkan support!"));
        }
        std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
        vkEnumeratePhysicalDevices(context, &deviceCount, physicalDevices.data());

        VkPhysicalDevice selectedPhysicalDevice = nullptr;
        for (VkPhysicalDevice physicalDevice : physicalDevices)
        {
            vkGetPhysicalDeviceProperties(physicalDevice, &m_physicalDeviceProperties);
            if (m_physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
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
        m_physicalDeviceHandle = selectedPhysicalDevice;

        vkGetPhysicalDeviceProperties(m_physicalDeviceHandle, &m_physicalDeviceProperties);
        APP_INFO("NAME {}", m_physicalDeviceProperties.deviceName);
        vkGetPhysicalDeviceFeatures(m_physicalDeviceHandle, &m_physicalDeviceFeatures);
        vkGetPhysicalDeviceMemoryProperties(m_physicalDeviceHandle, &m_physicalDeviceMemoryProperties);
    }
    void VulkanDevice::CreateLogicalDevice(){

        m_deviceExtensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        QueueFamilyIndicesInfo queueFamilyIndices = FindQueueFamilies(m_physicalDeviceHandle);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {queueFamilyIndices.graphicsFamily.value(), queueFamilyIndices.presentFamily.value()};

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.emplace_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures = {};
        deviceFeatures.samplerAnisotropy = VK_TRUE;

        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(m_deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = m_deviceExtensions.data();

        if (vkCreateDevice(m_physicalDeviceHandle, &createInfo, nullptr, &m_logicalDeviceHandle) != VK_SUCCESS)
        {
            CHECK_ERROR(APP_ERROR_VALUE, APP_ERROR("Failed to create logical device!"));
        }

        vkGetDeviceQueue(m_logicalDeviceHandle, queueFamilyIndices.graphicsFamily.value(), 0, &m_graphicsQueueHandle);
        vkGetDeviceQueue(m_logicalDeviceHandle, queueFamilyIndices.presentFamily.value(), 0, &m_presentQueueHandle);

        APP_INFO("QUEUE GRAPHICS: {} , QUEUE PRESENT: {}", queueFamilyIndices.graphicsFamily.value(), queueFamilyIndices.presentFamily.value());
    }

    void VulkanDevice::CreateCommandPool(){
        QueueFamilyIndicesInfo queueFamilyIndices = FindPhysicalQueueFamilies();

        VkCommandPoolCreateInfo cmdPoolInfo = {};
        cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cmdPoolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
        cmdPoolInfo.flags =
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        VK_CHECK_RESULT(vkCreateCommandPool(m_logicalDeviceHandle, &cmdPoolInfo, nullptr, &m_commandPoolHandle));
    }

    QueueFamilyIndicesInfo VulkanDevice::FindQueueFamilies(VkPhysicalDevice device)
    {
        QueueFamilyIndicesInfo indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto &queueFamily : queueFamilies)
        {
            if (((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) && ((queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0))
            {
                indices.graphicsFamily = i;
            }
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surfaceHandle, &presentSupport); 
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
    SwapChainSupportInfo VulkanDevice::QuerySwapChainSupport(VkPhysicalDevice device)
    {
        SwapChainSupportInfo details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surfaceHandle, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surfaceHandle, &formatCount, nullptr);

        if (formatCount != 0)
        {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surfaceHandle, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surfaceHandle, &presentModeCount, nullptr);

        if (presentModeCount != 0)
        {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(
                device,
                m_surfaceHandle,
                &presentModeCount,
                details.presentModes.data());
        }
        return details;
    }
}