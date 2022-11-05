#ifndef _RENDERER_VULKAN_SWAP_CHAIN_H_
#define _RENDERER_VULKAN_SWAP_CHAIN_H_


#include "VulkanDevice.h"

namespace VulkanPathfinding{
class VulkanSwapChain
{
public:
    VulkanSwapChain(VulkanDevice &deviceRef, std::shared_ptr<VulkanSwapChain> previous);
    
    ~VulkanSwapChain();

    VulkanSwapChain(const VulkanSwapChain &) = delete;
    VulkanSwapChain &operator=(const VulkanSwapChain &) = delete;

    void Initialize();
    void Destroy();
 
    VkResult AcquireNextImage(uint32_t *imageIndex);
    VkResult SubmitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);

    VkRenderPass m_renderPass;
    size_t m_currentFrame;

    static int MAX_FRAMES_IN_FLIGHT;
    std::vector<VkFramebuffer> m_swapChainFramebuffers;

    VkExtent2D m_swapChainExtent;
private:
    void CreateSwapChain();
    void CreateImageViews();

    void CreateRenderPass();
    void CreateFramebuffers();
    void CreateSyncObjects();


    VkSwapchainKHR m_swapChain;
    std::shared_ptr<VulkanSwapChain> m_oldSwapChain;

    VkFormat m_swapChainImageFormat;

    std::vector<VkImage> m_swapChainImages;
    std::vector<VkImageView> m_swapChainImageViews;

    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;

    std::vector<VkFence> m_inFlightFences;
    std::vector<VkFence> m_imagesInFlight;
    VulkanDevice& m_deviceRef;
};
}
#endif