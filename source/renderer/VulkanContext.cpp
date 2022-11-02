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

        for (auto& frameBuffer: m_vulkanFrameBuffers){
            vkDestroyFramebuffer(m_vulkanLogicalDeviceHandle, frameBuffer, nullptr);
        }

        vkDestroyRenderPass(m_vulkanLogicalDeviceHandle,m_vulkanRenderPassHandle,nullptr);

        for(auto& fence: m_vulkanWaitFences){
            vkDestroyFence(m_vulkanLogicalDeviceHandle, fence,nullptr);
        }

        vkDestroyCommandPool(m_vulkanLogicalDeviceHandle, m_vulkanCommandPoolHandle,nullptr);
        
        for (auto imageView : m_vulkanSwapChainImageViews)
        {
            vkDestroyImageView(m_vulkanLogicalDeviceHandle, imageView, nullptr);
        }

        vkDestroyDevice(m_vulkanLogicalDeviceHandle,nullptr);

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

        uint32_t queueFamilyCount;
        vkGetPhysicalDeviceQueueFamilyProperties(m_vulkanPhysicalDeviceHandle, &queueFamilyCount, nullptr);
        if(queueFamilyCount==0){
            CHECK_ERROR(APP_ERROR_VALUE, APP_ERROR("Failed to find any QueueFamily!"));
        }    
        m_queueFamilyProperties.resize(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(m_vulkanPhysicalDeviceHandle, &queueFamilyCount, m_queueFamilyProperties.data());


        m_deviceExtensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
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

        VkPhysicalDeviceFeatures deviceFeatures = {}; 

        VkDeviceCreateInfo createInfo = {}; 
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();                            
        createInfo.pEnabledFeatures = &deviceFeatures;                                     
        createInfo.enabledExtensionCount = static_cast<uint32_t>(m_deviceExtensions.size()); 
        createInfo.ppEnabledExtensionNames = m_deviceExtensions.data();                      

        
        if (vkCreateDevice(m_vulkanPhysicalDeviceHandle, &createInfo, nullptr, &m_vulkanLogicalDeviceHandle) != VK_SUCCESS)
        {
            CHECK_ERROR(APP_ERROR_VALUE, APP_ERROR("Failed to create logical device!"));
        }

        vkGetDeviceQueue(m_vulkanLogicalDeviceHandle, m_queueFamilyIndices.graphicsFamily.value(), 0, &m_vulkanGraphicsQueueHandle);
        vkGetDeviceQueue(m_vulkanLogicalDeviceHandle, m_queueFamilyIndices.presentFamily.value(), 0, &m_vulkanPresentationQueueHandle);

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

    void VulkanContext::InitSwapChain()
    {

        //Check what current swapchain supports, what features?
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_vulkanPhysicalDeviceHandle, m_surfaceHandle, &m_vulkanSwapchainCapabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_vulkanPhysicalDeviceHandle, m_surfaceHandle, &formatCount, nullptr);
        if (formatCount != 0)
        {
            m_vulkanSwapchainFormats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(m_vulkanPhysicalDeviceHandle, m_surfaceHandle, &formatCount, m_vulkanSwapchainFormats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(m_vulkanPhysicalDeviceHandle, m_surfaceHandle, &presentModeCount, nullptr);
        if (presentModeCount != 0)
        {
            m_vulkanSwapchainPresentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(m_vulkanPhysicalDeviceHandle, m_surfaceHandle, &presentModeCount, m_vulkanSwapchainPresentModes.data());
        }
        //

        //Choose those features based on support
        VkSurfaceFormatKHR surfaceFormat;
        for (auto &it : m_vulkanSwapchainFormats)
        {
            if (it.format == VK_FORMAT_B8G8R8A8_SRGB && it.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                if(Application::IsValidationEnabled())
                    APP_INFO("SRGB AND NONLINEAR SUPPORTED");
                
                surfaceFormat = it;
                break;
            }
        }

        VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

        for (auto &it : m_vulkanSwapchainPresentModes)
        {
            if (it == VK_PRESENT_MODE_MAILBOX_KHR) 
            {
                if (Application::IsValidationEnabled())
                    APP_INFO("MAILBOX PRESENT MODE SUPPORTED");
                presentMode = it;
                break;
            }
        }
        

        VkExtent2D extent;

        if (m_vulkanSwapchainCapabilities.currentExtent.width != UINT32_MAX)
        {
            extent = m_vulkanSwapchainCapabilities.currentExtent;
        }
        else
        {
            auto[width,height] = Application::GetWindow()->GetWindowSize();
            VkExtent2D actualExtent = {static_cast<uint32_t>(width),
                                       static_cast<uint32_t>(height)};

            actualExtent.width = std::max(m_vulkanSwapchainCapabilities.minImageExtent.width, std::min(m_vulkanSwapchainCapabilities.maxImageExtent.width, actualExtent.width));
            actualExtent.height = std::max(m_vulkanSwapchainCapabilities.minImageExtent.height, std::min(m_vulkanSwapchainCapabilities.maxImageExtent.height, actualExtent.height));
            extent = actualExtent;
        }

        uint32_t imageCount = m_vulkanSwapchainCapabilities.minImageCount + 1;

        if (m_vulkanSwapchainCapabilities.maxImageCount > 0 && imageCount > m_vulkanSwapchainCapabilities.maxImageCount)
        {
            imageCount = m_vulkanSwapchainCapabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = m_surfaceHandle;                       
        createInfo.minImageCount = imageCount;                    
        createInfo.imageFormat = surfaceFormat.format;              
        createInfo.imageColorSpace = surfaceFormat.colorSpace;      
        createInfo.imageExtent = extent;                            
        createInfo.imageArrayLayers = 1;                            
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;  


        //

        uint32_t queueFamilies[] = {m_queueFamilyIndices.graphicsFamily.value(), m_queueFamilyIndices.presentFamily.value()};
        if (m_queueFamilyIndices.graphicsFamily != m_queueFamilyIndices.presentFamily) 
        {                                                   
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilies;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        createInfo.preTransform = m_vulkanSwapchainCapabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;           
        createInfo.presentMode = presentMode;                                     
        createInfo.clipped = VK_TRUE;                                   
        createInfo.oldSwapchain = VK_NULL_HANDLE;                       

        m_vulkanCurrentSwapChainImageFormat = surfaceFormat.format;
        m_vulkanCurrentSwapChainExtent = extent;

        if (vkCreateSwapchainKHR(m_vulkanLogicalDeviceHandle, &createInfo, nullptr, &m_vulkanSwapChainHandle) != VK_SUCCESS)
        {
            CHECK_ERROR(APP_ERROR_VALUE, APP_ERROR("Failed to create SwapChain!"));
        }

        
        m_vulkanSwapChainImagesCount = imageCount;
        vkGetSwapchainImagesKHR(m_vulkanLogicalDeviceHandle, m_vulkanSwapChainHandle, &m_vulkanSwapChainImagesCount, nullptr);
        m_vulkanSwapChainImages.resize(m_vulkanSwapChainImagesCount);
        vkGetSwapchainImagesKHR(m_vulkanLogicalDeviceHandle, m_vulkanSwapChainHandle, &m_vulkanSwapChainImagesCount, m_vulkanSwapChainImages.data());
    }

    void VulkanContext::CreateImageViews(){
        m_vulkanSwapChainImageViews.resize(m_vulkanSwapChainImages.size());
        for (size_t i = 0; i < m_vulkanSwapChainImages.size(); i++)
        {
            VkImageViewCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = m_vulkanSwapChainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; 
            createInfo.format = m_vulkanCurrentSwapChainImageFormat;  

            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY; 
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY; 
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; 
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(m_vulkanLogicalDeviceHandle, &createInfo, nullptr, &m_vulkanSwapChainImageViews[i]) != VK_SUCCESS)
            {
                CHECK_ERROR(APP_ERROR_VALUE, APP_ERROR("Failed to create image views!"));
            }
        }
    }
    void VulkanContext::CreateCommandPools()
    {
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = m_queueFamilyIndices.graphicsFamily.value(); 

        if (vkCreateCommandPool(m_vulkanLogicalDeviceHandle, &poolInfo, nullptr, &m_vulkanCommandPoolHandle) != VK_SUCCESS)
        {
            CHECK_ERROR(APP_ERROR_VALUE, APP_ERROR("Failed to create command pool!"));
        }
    }
    void VulkanContext::CreateCommandBuffers(){
        m_vulkanCommandBuffers.resize(m_vulkanSwapChainImages.size());
        
        VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
        commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateInfo.commandPool = m_vulkanCommandPoolHandle;
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocateInfo.commandBufferCount = static_cast<uint32_t>(m_vulkanCommandBuffers.size());
        VK_CHECK_RESULT(
            vkAllocateCommandBuffers(m_vulkanLogicalDeviceHandle, &commandBufferAllocateInfo, m_vulkanCommandBuffers.data())
            ,APP_ERROR("Failed to create command buffers!")
            )

        APP_WARN("IMAGE SIZE: {} COMMAND BUFFER SIZE: {}", m_vulkanSwapChainImages.size(), m_vulkanCommandBuffers.size());

    }
    void VulkanContext::CreateSynchronizationObjects(){
  
        VkFenceCreateInfo fenceCreateInfo{};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        m_vulkanWaitFences.resize(m_vulkanCommandBuffers.size());
        for (auto &fence : m_vulkanWaitFences)
        {
            VK_CHECK_RESULT(vkCreateFence(m_vulkanLogicalDeviceHandle, &fenceCreateInfo, nullptr, &fence), APP_ERROR("Failed to create vulkan fences!"));
        }
    }

    void VulkanContext::SetupRenderPass(){
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = m_vulkanCurrentSwapChainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;

        VK_CHECK_RESULT(vkCreateRenderPass(m_vulkanLogicalDeviceHandle, &renderPassInfo, nullptr, &m_vulkanRenderPassHandle), APP_ERROR("Failed to create render pass!"));
    }

    void VulkanContext::CreateFrameBuffers(){
        m_vulkanFrameBuffers.resize(m_vulkanSwapChainImages.size());
    
    }
}