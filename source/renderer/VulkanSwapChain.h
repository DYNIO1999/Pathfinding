#ifndef _RENDERER_VULKAN_SWAP_CHAIN_H_
#define _RENDERER_VULKAN_SWAP_CHAIN_H_


#include "VulkanDevice.h"

namespace VulkanPathfinding{
class VulkanSwapChain
{
public:
    VulkanSwapChain(VulkanDevice &deviceRef);
    VulkanSwapChain(VulkanDevice &deviceRef, std::shared_ptr<VulkanSwapChain> previous);
    
    ~VulkanSwapChain();

    VulkanSwapChain(const VulkanSwapChain &) = delete;
    VulkanSwapChain &operator=(const VulkanSwapChain &) = delete;

 
 
    VkResult AcquireNextImage(uint32_t *imageIndex);
    VkResult SubmitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);
    VkRenderPass RenderPassHandle(){return m_renderPass;}
    size_t CurrentFrame(){return m_currentFrame;}
    VkFramebuffer AcquireFrameBuffer(size_t index){
        return m_swapChainFramebuffers[index];
    }

    std::pair < uint32_t, uint32_t> Size(){return {m_swapChainExtent.width,m_swapChainExtent.height};}
    VkExtent2D Extent(){return m_swapChainExtent;}
    
    static int MAX_FRAMES_IN_FLIGHT;

private:
    VkExtent2D m_swapChainExtent;
    std::vector<VkFramebuffer> m_swapChainFramebuffers;

    size_t m_currentFrame{0};
    VkRenderPass m_renderPass;
    
    void Initialize();
    void Destroy();

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