#include "VulkanAllocator.h"
#include "VulkanContext.h"
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

namespace Pathfinding
{
    VulkanAllocator::VulkanAllocator(VulkanDevice &device):m_deviceRef(device)
    {
        auto instance = VulkanContext::InstanceHandle(); 

        VmaVulkanFunctions vulkanFunctions = {};
        vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
        vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

        VmaAllocatorCreateInfo allocatorCreateInfo = {};
        allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_3;
        allocatorCreateInfo.instance = instance;
        allocatorCreateInfo.physicalDevice = m_deviceRef.PhysicalDeviceHandle();
        allocatorCreateInfo.device = m_deviceRef.LogicalDeviceHandle();
        allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

        VK_CHECK_RESULT(vmaCreateAllocator(&allocatorCreateInfo, &m_allocatorHandle));
    }

    VulkanAllocator::~VulkanAllocator()
    {

        uint32_t heapIndex = 0;

        VmaBudget budgets[VK_MAX_MEMORY_HEAPS];
        vmaGetHeapBudgets(m_allocatorHandle, budgets);

        APP_ERROR("VMA ALLOCATOR: has {} allocations taking {} bytes",
               budgets[heapIndex].statistics.allocationCount,
               budgets[heapIndex].statistics.allocationBytes);

        vmaDestroyAllocator(m_allocatorHandle);
    }

    VmaAllocation VulkanAllocator::AllocateBuffer(VkBufferCreateInfo *createInfo, VmaMemoryUsage usage, VkBuffer *outBuffer)
    {
        VmaAllocationCreateInfo allocationCreateInfo = {};
        allocationCreateInfo.usage = usage;

        VmaAllocation allocation;
        VmaAllocationInfo allocationInfo;
        VK_CHECK_RESULT(vmaCreateBuffer(m_allocatorHandle, createInfo, &allocationCreateInfo, outBuffer, &allocation, &allocationInfo));
        return allocation;
    }

    void VulkanAllocator::DestroyBuffer(VkBuffer buffer, VmaAllocation allocation)
    {
        vmaDestroyBuffer(m_allocatorHandle, buffer, allocation);
    }


    void *VulkanAllocator::MapMemory(VmaAllocation allocation)
    {
        void *memory;
        vmaMapMemory(m_allocatorHandle, allocation, &memory);
        return memory;
    }

    void VulkanAllocator::UnmapMemory(VmaAllocation allocation)
    {
        vmaUnmapMemory(m_allocatorHandle, allocation);
    }

    VmaAllocation VulkanAllocator::AllocateImage(VkImageCreateInfo *createInfo, VmaMemoryUsage usage, VkImage *outImage)
    {
        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.usage = usage;

        VmaAllocation allocation;
        VK_CHECK_RESULT(vmaCreateImage(m_allocatorHandle, createInfo, &allocCreateInfo, outImage, &allocation, nullptr));

        return allocation;
    }
    void VulkanAllocator::DestroyImage(VkImage image, VmaAllocation allocation)
    {
        vmaDestroyImage(m_allocatorHandle, image, allocation);
    }
} 
