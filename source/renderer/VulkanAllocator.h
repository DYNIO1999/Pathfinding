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

        VmaAllocation AllocateBuffer(VkBufferCreateInfo *create_info, uint32_t flags, VkBuffer *buffer);
        void DestroyBuffer(VkBuffer buffer, VmaAllocation allocation);

        void *MapMemory(VmaAllocation allocation);
        void UnmapMemory(VmaAllocation allocation);

    private:
        VulkanDevice& m_deviceRef;
        VmaAllocator m_allocatorHandle;
    };
}

#endif