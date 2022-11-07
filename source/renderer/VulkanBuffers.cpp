#include "VulkanBuffers.h"
#include "../core/Logger.h"

namespace Pathfinding{


    VulkanVertexBuffer::VulkanVertexBuffer(VulkanDevice &device, VulkanAllocator &allocator, void *data, uint32_t sizeInBytes)
        : m_deviceRef(device),
          m_allocatorRef(allocator)
    {
        Create(data, sizeInBytes);
    }
    VulkanVertexBuffer::~VulkanVertexBuffer()
    {
        m_allocatorRef.DestroyBuffer(m_bufferHandle, m_allocationHandle);
    }

    void VulkanVertexBuffer::Create(void *data, uint32_t sizeInBytes)
    {

        VkBufferCreateInfo bufferCreateInfo = {};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferCreateInfo.size = sizeInBytes;

        m_allocationHandle = m_allocatorRef.AllocateBuffer(&bufferCreateInfo, 0, &m_bufferHandle);

        VkBuffer stagingBuffer;
        VkBufferCreateInfo stagingBufferCreateInfo = {};
        stagingBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        stagingBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        stagingBufferCreateInfo.size = sizeInBytes;

        VmaAllocation stagingBufferAllocation = m_allocatorRef.AllocateBuffer(&stagingBufferCreateInfo,VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,&stagingBuffer);

        void *staging_buffer_memory = m_allocatorRef.MapMemory(stagingBufferAllocation);
        memcpy(staging_buffer_memory, data, sizeInBytes);
        m_allocatorRef.UnmapMemory(stagingBufferAllocation);

        

        VkCommandPool cmdPool;
        VkCommandPoolCreateInfo cmdPoolCreateInfo = {};
        cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        cmdPoolCreateInfo.queueFamilyIndex = m_deviceRef.GraphicsQueueFamilyIndex();
        VK_CHECK_RESULT(vkCreateCommandPool(m_deviceRef.LogicalDeviceHandle(), &cmdPoolCreateInfo, nullptr, &cmdPool));

        VkCommandBuffer cmdBuffer;
        VkCommandBufferAllocateInfo cmdBufferAllocateInfo = {};
        cmdBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmdBufferAllocateInfo.commandPool = cmdPool;
        cmdBufferAllocateInfo.commandBufferCount = 1;
        cmdBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        VK_CHECK_RESULT(vkAllocateCommandBuffers(m_deviceRef.LogicalDeviceHandle(), &cmdBufferAllocateInfo, &cmdBuffer));

        VkCommandBufferBeginInfo commandBufferBeginInfo = {};
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(cmdBuffer, &commandBufferBeginInfo);
        VkBufferCopy bufferCopy = {};
        bufferCopy.size = sizeInBytes;
        vkCmdCopyBuffer(cmdBuffer, stagingBuffer, m_bufferHandle, 1, &bufferCopy);
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
        m_allocatorRef.DestroyBuffer(stagingBuffer, stagingBufferAllocation );
        vkDestroyFence(m_deviceRef.LogicalDeviceHandle(), fence, nullptr);
    }

    VulkanIndexBuffer::VulkanIndexBuffer(VulkanDevice &device, VulkanAllocator &allocator, void *data, uint32_t sizeInBytes)
        : m_deviceRef(device),
          m_allocatorRef(allocator)
    {
        Create(data, sizeInBytes);
    }
    VulkanIndexBuffer::~VulkanIndexBuffer()
    {
        m_allocatorRef.DestroyBuffer(m_bufferHandle, m_allocationHandle);
    }

    void VulkanIndexBuffer::Create(void *data, uint32_t sizeInBytes){

        VkBufferCreateInfo bufferCreateInfo = {};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferCreateInfo.size = sizeInBytes;

        m_indicesCount = sizeInBytes / sizeof(uint32_t);
        m_allocationHandle = m_allocatorRef.AllocateBuffer(&bufferCreateInfo, 0, &m_bufferHandle);

        VkBuffer stagingBuffer;
        VkBufferCreateInfo stagingBufferCreateInfo = {};
        stagingBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        stagingBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        stagingBufferCreateInfo.size = sizeInBytes;

        VmaAllocation stagingBufferAllocation = m_allocatorRef.AllocateBuffer(&stagingBufferCreateInfo, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT, &stagingBuffer);

        void *staging_buffer_memory = m_allocatorRef.MapMemory(stagingBufferAllocation);
        memcpy(staging_buffer_memory, data, sizeInBytes);
        m_allocatorRef.UnmapMemory(stagingBufferAllocation);

        VkCommandPool cmdPool;
        VkCommandPoolCreateInfo cmdPoolCreateInfo = {};
        cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        cmdPoolCreateInfo.queueFamilyIndex = m_deviceRef.GraphicsQueueFamilyIndex();
        VK_CHECK_RESULT(vkCreateCommandPool(m_deviceRef.LogicalDeviceHandle(), &cmdPoolCreateInfo, nullptr, &cmdPool));

        VkCommandBuffer cmdBuffer;
        VkCommandBufferAllocateInfo cmdBufferAllocateInfo = {};
        cmdBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmdBufferAllocateInfo.commandPool = cmdPool;
        cmdBufferAllocateInfo.commandBufferCount = 1;
        cmdBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        VK_CHECK_RESULT(vkAllocateCommandBuffers(m_deviceRef.LogicalDeviceHandle(), &cmdBufferAllocateInfo, &cmdBuffer));

        VkCommandBufferBeginInfo commandBufferBeginInfo = {};
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(cmdBuffer, &commandBufferBeginInfo);
        VkBufferCopy bufferCopy = {};
        bufferCopy.size = sizeInBytes;
        vkCmdCopyBuffer(cmdBuffer, stagingBuffer, m_bufferHandle, 1, &bufferCopy);
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
        m_allocatorRef.DestroyBuffer(stagingBuffer, stagingBufferAllocation);
        vkDestroyFence(m_deviceRef.LogicalDeviceHandle(), fence, nullptr);
    }
}