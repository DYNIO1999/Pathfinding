#ifndef _RENDERER_VULKAN_ALLOCATOR_H_
#define _RENDERER_VULKAN_ALLOCATOR_H_

#include <vk_mem_alloc.h>
#include "VulkanDevice.h"

namespace Pathfinding
{
    class VulkanAllocator
    {
    public:
        VulkanAllocator(VulkanDevice &device);
        ~VulkanAllocator();

        VmaAllocator AllocatorHandle() { return m_allocatorHandle;}

        VmaAllocation AllocateBuffer(VkBufferCreateInfo *createInfo, VmaMemoryUsage usage, VkBuffer *outBuffer);
        void DestroyBuffer(VkBuffer buffer, VmaAllocation allocation);

        void *MapMemory(VmaAllocation allocation);
        void UnmapMemory(VmaAllocation allocation);

        VmaAllocation AllocateImage(VkImageCreateInfo* createInfo, VmaMemoryUsage usage, VkImage *outImage);
        void DestroyImage(VkImage image, VmaAllocation allocation);
        void ShowAllocatedMemory();
    private: 
        VulkanDevice &m_deviceRef;
        VmaAllocator m_allocatorHandle;
    };
}

#endif