#include "VulkanContext.h"
#include "../core/Application.h"

namespace VulkanPathfinding{
    bool VulkanContext::framebufferResized =false;
    std::vector<char> VulkanContext::ReadFile(const std::string &path)
    {
        std::vector<char> shaderSource;
        std::ifstream shaderFile(path, std::ios::in | std::ios::binary);
        if (shaderFile)
        {
            shaderFile.seekg(0, std::ios::end);
            shaderSource.resize(shaderFile.tellg());
            shaderFile.seekg(0, std::ios::beg);
            shaderFile.read(shaderSource.data(), shaderSource.size());
            shaderFile.close();
        }
        else
        {
            CHECK_ERROR(APP_ERROR_VALUE, APP_ERROR("Cant load shader!"));
        }
        return shaderSource;
    }

    VkShaderModule VulkanContext::CreateShaderModule(const std::vector<char> &code)
    {
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

        VkShaderModule shaderModule;
        VK_CHECK_RESULT(vkCreateShaderModule(m_vulkanLogicalDeviceHandle, &createInfo, nullptr, &shaderModule), APP_ERROR("Failed to create shader module!"));

        return shaderModule;
    }

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

        vkDeviceWaitIdle(m_vulkanLogicalDeviceHandle); //waits on device to be idle

        vkDestroyPipeline(m_vulkanLogicalDeviceHandle, m_vulkanGraphicsPipelineHandle,nullptr);

        vkDestroyPipelineLayout(m_vulkanLogicalDeviceHandle, m_vulkanPipelineLayoutHandle, nullptr);

        for (auto& frameBuffer: m_vulkanFrameBuffers){
            vkDestroyFramebuffer(m_vulkanLogicalDeviceHandle, frameBuffer, nullptr);
        }

        vkDestroyRenderPass(m_vulkanLogicalDeviceHandle,m_vulkanRenderPassHandle,nullptr);

        for(auto& fence: m_queueCompleteFences){
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

        APP_INFO("QUEUE GRAPHICS: {} , QUEUE PRESENT: {}", m_queueFamilyIndices.graphicsFamily.value(), m_queueFamilyIndices.presentFamily.value());
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

        VkPresentModeKHR presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR; //If V-Sync off
        //VK_PRESENT_MODE_FIFO_KHR if VSync on
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

        uint32_t imageCount = m_vulkanSwapchainCapabilities.minImageCount;

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
        APP_ERROR("SIZE OF SWAPCHAIN IMAGES: {}", imageCount);
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
        m_vulkanCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        
        VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
        commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateInfo.commandPool = m_vulkanCommandPoolHandle;
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocateInfo.commandBufferCount = static_cast<uint32_t>(m_vulkanCommandBuffers.size());
        VK_CHECK_RESULT(
            vkAllocateCommandBuffers(m_vulkanLogicalDeviceHandle, &commandBufferAllocateInfo, m_vulkanCommandBuffers.data())
            ,APP_ERROR("Failed to create command buffers!")
            )

        APP_WARN("IMAGE SIZE: {} COMMAND BUFFER SIZE: {}", MAX_FRAMES_IN_FLIGHT, MAX_FRAMES_IN_FLIGHT);
    }
    void VulkanContext::CreateSynchronizationObjects(){

        VkSemaphoreCreateInfo semaphoreCreateInfo = {};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreCreateInfo.pNext = nullptr;
        
        m_presentCompleteSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
        m_renderCompleteSemaphore.resize(MAX_FRAMES_IN_FLIGHT);



        VkFenceCreateInfo fenceCreateInfo{};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        m_queueCompleteFences.resize(MAX_FRAMES_IN_FLIGHT);
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            VK_CHECK_RESULT(vkCreateSemaphore(m_vulkanLogicalDeviceHandle, &semaphoreCreateInfo, nullptr, &m_presentCompleteSemaphore[i]), APP_ERROR("Failed to create vulkan semaphore!"));
            VK_CHECK_RESULT(vkCreateSemaphore(m_vulkanLogicalDeviceHandle, &semaphoreCreateInfo, nullptr, &m_renderCompleteSemaphore[i]), APP_ERROR("Failed to create vulkan semaphore!"));
            VK_CHECK_RESULT(vkCreateFence(m_vulkanLogicalDeviceHandle, &fenceCreateInfo, nullptr, &m_queueCompleteFences[i]), APP_ERROR("Failed to create vulkan fences!"));
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

    void VulkanContext::CreateGraphicsPipeline()
    {   
        auto vertexShader = ReadFile("../shaders/test.vert.spv");
        auto fragmentShader = ReadFile("../shaders/test.frag.spv");

        VkShaderModule vertexShaderModule = CreateShaderModule(vertexShader);
        VkShaderModule fragmentShaderModule = CreateShaderModule(fragmentShader);

        VkPipelineShaderStageCreateInfo shaderStages[2];
        shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStages[0].module = vertexShaderModule;
        shaderStages[0].pName = "main";
        shaderStages[0].flags = 0;
        shaderStages[0].pNext = nullptr;
        shaderStages[0].pSpecializationInfo = nullptr;


        shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages[1].module = fragmentShaderModule;
        shaderStages[1].pName = "main";
        shaderStages[1].flags = 0;
        shaderStages[1].pNext = nullptr;
        shaderStages[1].pSpecializationInfo = nullptr;

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.vertexAttributeDescriptionCount = 0;

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pushConstantRangeCount = 0;

        VK_CHECK_RESULT(vkCreatePipelineLayout(m_vulkanLogicalDeviceHandle, &pipelineLayoutInfo, nullptr, &m_vulkanPipelineLayoutHandle), APP_ERROR("Failed to create pipeline layout!"));

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = m_vulkanPipelineLayoutHandle;
        pipelineInfo.renderPass = m_vulkanRenderPassHandle;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        VK_CHECK_RESULT(vkCreateGraphicsPipelines(m_vulkanLogicalDeviceHandle, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_vulkanGraphicsPipelineHandle), APP_ERROR("Failed to create graphics pipeline!"));

        vkDestroyShaderModule(m_vulkanLogicalDeviceHandle, fragmentShaderModule, nullptr);
        vkDestroyShaderModule(m_vulkanLogicalDeviceHandle, vertexShaderModule, nullptr);


    }

    void VulkanContext::CreateFrameBuffers(){
        m_vulkanFrameBuffers.resize(m_vulkanSwapChainImageViews.size());

        for (size_t i = 0; i < m_vulkanSwapChainImageViews.size(); i++)
        {
            VkImageView attachments[] = {
                m_vulkanSwapChainImageViews[i]};

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_vulkanRenderPassHandle;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = m_vulkanCurrentSwapChainExtent.width;
            framebufferInfo.height = m_vulkanCurrentSwapChainExtent.height;
            framebufferInfo.layers = 1;

            VK_CHECK_RESULT(vkCreateFramebuffer(m_vulkanLogicalDeviceHandle, &framebufferInfo, nullptr, &m_vulkanFrameBuffers[i]), APP_ERROR("Failed to create framebuffer!"));
        }
    }


    void VulkanContext::RecordCommandBuffers()
    {
        VkCommandBufferBeginInfo cmdBufInfo = {};
        cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cmdBufInfo.pNext = nullptr;

        VkClearValue clearColor = {{{1.0f, 1.0f, 0.5f, 1.0f}}};

        APP_WARN("SIZE : {} , {}", m_vulkanCurrentSwapChainExtent.width, m_vulkanCurrentSwapChainExtent.height);
        VkRenderPassBeginInfo renderPassBeginInfo = {};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.pNext = nullptr;
        renderPassBeginInfo.renderPass = m_vulkanRenderPassHandle;
        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.renderArea.extent.width = m_vulkanCurrentSwapChainExtent.width;
        renderPassBeginInfo.renderArea.extent.height = m_vulkanCurrentSwapChainExtent.height;
        renderPassBeginInfo.clearValueCount = 1;
        renderPassBeginInfo.pClearValues = &clearColor;

        for (int32_t i = 0; i < static_cast<int32_t>(m_vulkanCommandBuffers.size()); i++)
        {
            APP_ERROR("CHECK");
            renderPassBeginInfo.framebuffer = m_vulkanFrameBuffers[i];
            VK_CHECK_RESULT(vkBeginCommandBuffer(m_vulkanCommandBuffers[i], &cmdBufInfo), APP_ERROR("Failed to create BeginCommandBuffer"));
            vkCmdBeginRenderPass(m_vulkanCommandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindPipeline(m_vulkanCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_vulkanGraphicsPipelineHandle);
            
            VkViewport viewport = {};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.height = (float)m_vulkanCurrentSwapChainExtent.height;
            viewport.width =(float)m_vulkanCurrentSwapChainExtent.width; 
            viewport.minDepth = (float)0.0f;
            viewport.maxDepth = (float)1.0f;
            vkCmdSetViewport(m_vulkanCommandBuffers[i], 0, 1, &viewport);

            VkRect2D scissor = {};
            scissor.extent.width = m_vulkanCurrentSwapChainExtent.width;
            scissor.extent.height = m_vulkanCurrentSwapChainExtent.height;
            scissor.offset.x = 0;
            scissor.offset.y = 0;
            vkCmdSetScissor(m_vulkanCommandBuffers[i], 0, 1, &scissor);

            vkCmdDraw(m_vulkanCommandBuffers[i], 3, 1, 0, 0);

            vkCmdEndRenderPass(m_vulkanCommandBuffers[i]);
            VK_CHECK_RESULT(vkEndCommandBuffer(m_vulkanCommandBuffers[i]), APP_ERROR("Failed to create EndCommandBuffer"));
        }
    }

    void VulkanContext::RecreateSwapChain(){

        vkDeviceWaitIdle(m_vulkanLogicalDeviceHandle);


        
        for (auto &imageViews : m_vulkanSwapChainImageViews)
        {
            vkDestroyImageView(m_vulkanLogicalDeviceHandle, imageViews, nullptr);
        }
        for (auto &frameBuffer : m_vulkanFrameBuffers)
        {
            vkDestroyFramebuffer(m_vulkanLogicalDeviceHandle, frameBuffer, nullptr);
        }

        vkDestroySwapchainKHR(m_vulkanLogicalDeviceHandle, m_vulkanSwapChainHandle, nullptr);

        vkFreeCommandBuffers(m_vulkanLogicalDeviceHandle, m_vulkanCommandPoolHandle, static_cast<uint32_t>(m_vulkanCommandBuffers.size()), m_vulkanCommandBuffers.data());
        
        InitSwapChain();
        CreateImageViews();
        CreateFrameBuffers();
        CreateCommandBuffers();
        RecordCommandBuffers();
    }

    void VulkanContext::Draw(){

        VK_CHECK_RESULT(vkWaitForFences(m_vulkanLogicalDeviceHandle, 1, &m_queueCompleteFences[m_currentActiveFrameBuffer], VK_TRUE, UINT64_MAX), APP_ERROR("Failed to wait for Vulkan Fence!"));
        uint32_t imageIndex;
        VkResult acquire = vkAcquireNextImageKHR(m_vulkanLogicalDeviceHandle, m_vulkanSwapChainHandle, UINT64_MAX, m_presentCompleteSemaphore[m_currentActiveFrameBuffer], (VkFence) nullptr, &imageIndex);
        if ((acquire == VK_ERROR_OUT_OF_DATE_KHR) || (acquire == VK_SUBOPTIMAL_KHR))
        {
                if (acquire == VK_ERROR_OUT_OF_DATE_KHR)
                {
                    RecreateSwapChain();
                }
                return;
        }else
        {
            VK_CHECK_RESULT(acquire, APP_ERROR("Failed to acquire next image from Swap Chain"));
        }
        
        VK_CHECK_RESULT(vkResetFences(m_vulkanLogicalDeviceHandle, 1, &m_queueCompleteFences[m_currentActiveFrameBuffer]), APP_ERROR("Failed to reset Vulkan Fence!"));


        // Pipeline stage at which the queue submission will wait (via pWaitSemaphores)
        VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        // The submit info structure specifies a command buffer queue submission batch
        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pWaitDstStageMask = &waitStageMask;               // Pointer to the list of pipeline stages that the semaphore waits will occur at
        submitInfo.waitSemaphoreCount = 1;                           // One wait semaphore
        submitInfo.signalSemaphoreCount = 1;                         // One signal semaphore
        submitInfo.pCommandBuffers = &m_vulkanCommandBuffers[m_currentActiveFrameBuffer]; // Command buffers(s) to execute in this batch (submission)
        submitInfo.commandBufferCount = 1;                           // One command buffer

        submitInfo.pWaitSemaphores = &m_presentCompleteSemaphore[m_currentActiveFrameBuffer];  // Semaphore(s) to wait upon before the submitted command buffer starts executing
        submitInfo.pSignalSemaphores = &m_renderCompleteSemaphore[m_currentActiveFrameBuffer]; // Semaphore(s) to be signaled when command buffers have completed

        // Submit to the graphics queue passing a wait fence
        VK_CHECK_RESULT(vkQueueSubmit(m_vulkanGraphicsQueueHandle, 1, &submitInfo, m_queueCompleteFences[m_currentActiveFrameBuffer]), APP_ERROR("Failed to submit to Vulkan Queue!"));

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pNext = NULL;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &m_vulkanSwapChainHandle;
        presentInfo.pImageIndices = &imageIndex;
        // Check if a wait semaphore has been specified to wait for before presenting the image
        if (m_renderCompleteSemaphore[m_currentActiveFrameBuffer] != VK_NULL_HANDLE)
        {
            presentInfo.pWaitSemaphores = &m_renderCompleteSemaphore[m_currentActiveFrameBuffer];
            presentInfo.waitSemaphoreCount = 1;
        }


        VkResult result =  vkQueuePresentKHR(m_vulkanGraphicsQueueHandle, &presentInfo);
        if((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR) || framebufferResized)
        {
            framebufferResized =false;
            RecreateSwapChain();
            if (result == VK_ERROR_OUT_OF_DATE_KHR)
            {
                return;
            }
        }
        m_currentActiveFrameBuffer = (m_currentActiveFrameBuffer + 1) % MAX_FRAMES_IN_FLIGHT;
    }
}