#include "VulkanBuffer.h"
#include "VulkanInitializers.h"

#include "../core/Logger.h"
namespace Pathfinding
{

    VulkanBufferBuilder&& VulkanBuffer::Create(VulkanDevice &device, VulkanAllocator &allocator) {
        return std::move(VulkanBufferBuilder{device,allocator});
    }

    VulkanBuffer::VulkanBuffer(VulkanDevice &device, VulkanAllocator &allocator):
    m_deviceRef(device),
    m_allocatorRef(allocator)
    {
        APP_ERROR("CREATED BUFFER");
    }

    VulkanBuffer::~VulkanBuffer()
    {
        if(m_allocationHandle){
            APP_ERROR("DESTROYED BUFFER");
            m_allocatorRef.DestroyBuffer(m_bufferHandle,m_allocationHandle);
        }   
    }
    void VulkanBuffer::CreateVertexBuffer()
    {

        APP_ERROR(m_sizeInBytes);
        m_bufferCreateInfo = VulkanInitializers::BufferCreateInfo(
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_SHARING_MODE_EXCLUSIVE, m_sizeInBytes);

        m_allocationHandle =m_allocatorRef.AllocateBuffer(&m_bufferCreateInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, &m_bufferHandle);

        //Staging buffer
        VkBufferCreateInfo bufferCreateInfo2{};

        bufferCreateInfo2 = VulkanInitializers::BufferCreateInfo(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                                 VK_SHARING_MODE_EXCLUSIVE,
                                                                 m_sizeInBytes);

        VmaAllocation stagingBufferAllocationHandle{nullptr};
        VkBuffer stagingBufferHandle{nullptr};
        void* stagingData{nullptr};

        stagingBufferAllocationHandle =
            m_allocatorRef.AllocateBuffer(
                &bufferCreateInfo2,
                VMA_MEMORY_USAGE_CPU_TO_GPU,
                &stagingBufferHandle);

        stagingData = m_allocatorRef.MapMemory(stagingBufferAllocationHandle);
        memcpy(stagingData, m_actualData, m_sizeInBytes);
        m_allocatorRef.UnmapMemory(stagingBufferAllocationHandle);

        VkCommandPool cmdPool;

        VkCommandPoolCreateInfo cmdPoolCreateInfo{};
        cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cmdPoolCreateInfo.queueFamilyIndex = m_deviceRef.GraphicsQueueFamilyIndex();
        cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

        VK_CHECK_RESULT(vkCreateCommandPool(m_deviceRef.LogicalDeviceHandle(), &cmdPoolCreateInfo, VK_NULL_HANDLE, &cmdPool));

        VkCommandBuffer cmdBuffer;

        VkCommandBufferAllocateInfo cmdBufferAllocateInfo{};
        cmdBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmdBufferAllocateInfo.commandPool = cmdPool;
        cmdBufferAllocateInfo.commandBufferCount = 1;
        cmdBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        vkAllocateCommandBuffers(m_deviceRef.LogicalDeviceHandle(), &cmdBufferAllocateInfo, &cmdBuffer);

        VkCommandBufferBeginInfo commandBufferBeginInfo = {};
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(cmdBuffer, &commandBufferBeginInfo);
        VkBufferCopy bufferCopy = {};
        bufferCopy.size = m_sizeInBytes;
        vkCmdCopyBuffer(cmdBuffer, stagingBufferHandle, m_bufferHandle, 1, &bufferCopy);
        vkEndCommandBuffer(cmdBuffer);

        VkFence fence;
        VkFenceCreateInfo fenceCreateInfo = {};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        VK_CHECK_RESULT(vkCreateFence(m_deviceRef.LogicalDeviceHandle(), &fenceCreateInfo, nullptr, &fence));

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmdBuffer;
        

        VK_CHECK_RESULT(vkQueueSubmit(m_deviceRef.GraphicsQueueHandle(), 1, &submitInfo, fence));
        vkWaitForFences(m_deviceRef.LogicalDeviceHandle(), 1, &fence, VK_TRUE, UINT64_MAX);

        vkDestroyCommandPool(m_deviceRef.LogicalDeviceHandle(), cmdPool, nullptr);
        m_allocatorRef.DestroyBuffer(stagingBufferHandle, stagingBufferAllocationHandle);
        vkDestroyFence(m_deviceRef.LogicalDeviceHandle(), fence, nullptr);
    }
}
