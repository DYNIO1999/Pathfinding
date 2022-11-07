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
        vmaDestroyAllocator(m_allocatorHandle);
    }

    VmaAllocation VulkanAllocator::AllocateBuffer(VkBufferCreateInfo *create_info, VkBuffer *buffer)
    {
        VmaAllocationCreateInfo allocationCreateInfo = {};
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        
        VmaAllocation allocation;
        VmaAllocationInfo allocationInfo;
        VK_CHECK_RESULT(vmaCreateBuffer(m_allocatorHandle, create_info, &allocationCreateInfo, buffer, &allocation, &allocationInfo));
        return allocation;
    }

    void VulkanAllocator::DestroyBuffer(VkBuffer buffer, VmaAllocation allocation)
    {
        VmaAllocationInfo allocationInfo;
        vmaGetAllocationInfo(m_allocatorHandle, allocation, &allocationInfo);
        vkDeviceWaitIdle(m_deviceRef.LogicalDeviceHandle());
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
} 
