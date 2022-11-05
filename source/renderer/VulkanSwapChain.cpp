#include "VulkanSwapChain.h"
#include "../core/Application.h"


namespace VulkanPathfinding
{
    int VulkanSwapChain::MAX_FRAMES_IN_FLIGHT =2;

    VulkanSwapChain::VulkanSwapChain(VulkanDevice &deviceRef, std::shared_ptr<VulkanSwapChain> previous) : m_oldSwapChain(previous), m_deviceRef(deviceRef)
    {
        Initialize();
        m_oldSwapChain = nullptr;
    }

    VulkanSwapChain::~VulkanSwapChain()
    {

    }

    void VulkanSwapChain::Destroy(){
        for (auto imageView : m_swapChainImageViews)
        {
            vkDestroyImageView(m_deviceRef.LogicalDeviceHandle(), imageView, nullptr);
        }
        m_swapChainImageViews.clear();

        if (m_swapChain != nullptr)
        {
            vkDestroySwapchainKHR(m_deviceRef.LogicalDeviceHandle(), m_swapChain, nullptr);
            m_swapChain = nullptr;
        }

        for (auto framebuffer : m_swapChainFramebuffers)
        {
            vkDestroyFramebuffer(m_deviceRef.LogicalDeviceHandle(), framebuffer, nullptr);
        }

        vkDestroyRenderPass(m_deviceRef.LogicalDeviceHandle(), m_renderPass, nullptr);

        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            vkDestroySemaphore(m_deviceRef.LogicalDeviceHandle(), m_renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(m_deviceRef.LogicalDeviceHandle(), m_imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(m_deviceRef.LogicalDeviceHandle(), m_inFlightFences[i], nullptr);
        }
    }

    void VulkanSwapChain::Initialize()
    {

        CreateSwapChain();
        CreateImageViews();
        CreateRenderPass();
        CreateFramebuffers();
        CreateSyncObjects();
    }

    void VulkanSwapChain::CreateSwapChain()
    {
        SwapChainSupportInfo swapChainSupportInfo = m_deviceRef.GetSwapChainSupport();

        VkSurfaceFormatKHR surfaceFormat;
        VkPresentModeKHR presentMode;
        VkExtent2D extent;

        for (auto &it : swapChainSupportInfo.formats)
        {
            if (it.format == VK_FORMAT_B8G8R8A8_SRGB && it.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                surfaceFormat = it;
                break;
            }
        }

        presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR; // If V-Sync off
        // VK_PRESENT_MODE_FIFO_KHR if VSync on

        for (auto &it : swapChainSupportInfo.presentModes)
        {

            if (it == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                presentMode = it;
                break;
            }
        }

        if (swapChainSupportInfo.capabilities.currentExtent.width == (uint32_t)-1)
        {
            auto [width,height]  =Application::GetWindow()->WindowSize();
            VkExtent2D actualExtent = {static_cast<uint32_t>(width),
                                       static_cast<uint32_t>(height)};
            extent.width = std::max(
                swapChainSupportInfo.capabilities.minImageExtent.width,
                std::min(swapChainSupportInfo.capabilities.maxImageExtent.width, actualExtent.width));
            extent.height = std::max(
                swapChainSupportInfo.capabilities.minImageExtent.height,
                std::min(swapChainSupportInfo.capabilities.maxImageExtent.height, actualExtent.height));
        }
        else
        {
            extent = swapChainSupportInfo.capabilities.currentExtent;
        }

        uint32_t imageCount = swapChainSupportInfo.capabilities.minImageCount + 1;
        if (swapChainSupportInfo.capabilities.maxImageCount > 0 &&
            imageCount > swapChainSupportInfo.capabilities.maxImageCount)
        {
            imageCount = swapChainSupportInfo.capabilities.maxImageCount;
        }


        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = m_deviceRef.SurfaceHandle();
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndicesInfo indices = m_deviceRef.FindPhysicalQueueFamilies();
        uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        if (indices.graphicsFamily != indices.presentFamily)
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;     
            createInfo.pQueueFamilyIndices = nullptr; 
        }
        createInfo.preTransform = swapChainSupportInfo.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        if(!m_oldSwapChain){
            createInfo.oldSwapchain = VK_NULL_HANDLE;
        }else{
            createInfo.oldSwapchain = m_oldSwapChain->m_swapChain;
        }
        if (vkCreateSwapchainKHR(m_deviceRef.LogicalDeviceHandle(), &createInfo, nullptr, &m_swapChain) != VK_SUCCESS)
        {
            CHECK_ERROR(APP_ERROR_VALUE, APP_ERROR("Failed to create SwapChain!"));
        }
        vkGetSwapchainImagesKHR(m_deviceRef.LogicalDeviceHandle(), m_swapChain, &imageCount, nullptr);
        m_swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(m_deviceRef.LogicalDeviceHandle(), m_swapChain, &imageCount, m_swapChainImages.data());

        m_swapChainImageFormat = surfaceFormat.format;
        m_swapChainExtent = extent;
    }
    void VulkanSwapChain::CreateImageViews()
    {
        m_swapChainImageViews.resize(m_swapChainImages.size());
        for (size_t i = 0; i < m_swapChainImages.size(); i++)
        {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = m_swapChainImages[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = m_swapChainImageFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(m_deviceRef.LogicalDeviceHandle(), &viewInfo, nullptr, &m_swapChainImageViews[i]) !=VK_SUCCESS)
            {
                CHECK_ERROR(APP_ERROR_VALUE, APP_ERROR("Failed to create Vulkan Image View!"));
            }
        }
    }

    void VulkanSwapChain::CreateRenderPass()
    {
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = m_swapChainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        
        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkSubpassDependency dependency = {};
        dependency.dstSubpass = 0;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.srcAccessMask = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(m_deviceRef.LogicalDeviceHandle(), &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS)
        {
            CHECK_ERROR(APP_ERROR_VALUE, APP_ERROR("Failed to create Vulkan Render Pass!"));
        }
    }
    void VulkanSwapChain::CreateFramebuffers()
    {
        m_swapChainFramebuffers.resize(m_swapChainImages.size());
        for (size_t i = 0; i < m_swapChainImages.size(); i++)
        {
            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_renderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = &m_swapChainImageViews[i];
            framebufferInfo.width = m_swapChainExtent.width;
            framebufferInfo.height = m_swapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(m_deviceRef.LogicalDeviceHandle(), &framebufferInfo, nullptr, &m_swapChainFramebuffers[i]) != VK_SUCCESS)
            {
                CHECK_ERROR(APP_ERROR_VALUE, APP_ERROR("Failed to create Vulkan Frame Buffer!"));
            }
        }
    }
    void VulkanSwapChain::CreateSyncObjects()
    {
        m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        APP_ERROR("SWAP CHAIN IMAGES: {}", m_swapChainImages.size());
        m_imagesInFlight.resize(m_swapChainImages.size(), VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {

            vkCreateSemaphore(m_deviceRef.LogicalDeviceHandle(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]);
            vkCreateSemaphore(m_deviceRef.LogicalDeviceHandle(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]);
            vkCreateFence(m_deviceRef.LogicalDeviceHandle(), &fenceInfo, nullptr, &m_inFlightFences[i]);
        }
    }

    VkResult VulkanSwapChain::AcquireNextImage(uint32_t *imageIndex)
    {
        vkWaitForFences(m_deviceRef.LogicalDeviceHandle(), 1, &m_inFlightFences[m_currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());

        VkResult result = vkAcquireNextImageKHR(
            m_deviceRef.LogicalDeviceHandle(),
            m_swapChain,
            std::numeric_limits<uint64_t>::max(),
            m_imageAvailableSemaphores[m_currentFrame], 
            VK_NULL_HANDLE,
            imageIndex);

        return result;
    }
    VkResult VulkanSwapChain::SubmitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex)
    {
        if (m_imagesInFlight[*imageIndex] != VK_NULL_HANDLE)
        {
            vkWaitForFences(m_deviceRef.LogicalDeviceHandle(), 1, &m_imagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);
        }
        m_imagesInFlight[*imageIndex] = m_inFlightFences[m_currentFrame];

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {m_imageAvailableSemaphores[m_currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = buffers;

        VkSemaphore signalSemaphores[] = {m_renderFinishedSemaphores[m_currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        vkResetFences(m_deviceRef.LogicalDeviceHandle(), 1, &m_inFlightFences[m_currentFrame]);
        if (vkQueueSubmit(m_deviceRef.GraphicsQueueHandle(), 1, &submitInfo, m_inFlightFences[m_currentFrame]) !=
            VK_SUCCESS)
        {
            CHECK_ERROR(APP_ERROR_VALUE, APP_ERROR("Failed to submit draw command buffer!"));
        }

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {m_swapChain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = imageIndex;

        auto result = vkQueuePresentKHR(m_deviceRef.PresentQueueHandle(), &presentInfo);

        m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

        return result;
    }
} 
