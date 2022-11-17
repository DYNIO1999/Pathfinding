#include "VulkanBuffer.h"

namespace Pathfinding
{

    VulkanBuffer::VulkanBuffer(
       VulkanDevice &device,
       VulkanAllocator &allocator,
       VulkanBufferType type,
       void *data,
       uint64_t sizeInBytes):
           m_device(device),
           m_allocator(allocator),
           m_type(type),
           m_actualData(data),
           m_sizeInBytes(sizeInBytes)
    {
        if(m_type == VulkanBufferType::VERTEX_BUFFER){
            CreateVertexBuffer();
        }else if(m_type == VulkanBufferType::INDEX_BUFFER){
            CreateIndexBuffer();
        }
    }
    VulkanBuffer::~VulkanBuffer(){
        if(m_bufferHandle && m_allocationHandle){
            m_allocator.DestroyBuffer(m_bufferHandle,m_allocationHandle);
        }
    }

    VkBufferCreateInfo VulkanBuffer::BufferCreateInfo(VkBufferUsageFlags usage, VkSharingMode sharingMode, uint32_t sizeInBytes)
    {
        VkBufferCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        info.usage = usage;
        info.size = sizeInBytes;
        info.sharingMode = sharingMode;
        return info;
    }

    void VulkanBuffer::CreateVertexBuffer(){

        VkBufferCreateInfo bufferCreateInfo = BufferCreateInfo(
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_SHARING_MODE_EXCLUSIVE, m_sizeInBytes);

        m_allocationHandle = m_allocator.AllocateBuffer(&bufferCreateInfo, VMA_MEMORY_USAGE_GPU_ONLY, &m_bufferHandle);

        VkBufferCreateInfo stagingbufferCreateInfo{};

        stagingbufferCreateInfo = BufferCreateInfo(
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, m_sizeInBytes);


        VkBuffer stagingBufferHandle;

        VmaAllocation stagingBufferAllocation =
            m_allocator.AllocateBuffer(
                &stagingbufferCreateInfo,
                VMA_MEMORY_USAGE_CPU_TO_GPU,
                &stagingBufferHandle);

        void* data;
        data= m_allocator.MapMemory(stagingBufferAllocation);
        memcpy(data, m_actualData, m_sizeInBytes);
        m_allocator.UnmapMemory(stagingBufferAllocation);
        
        Transfer(stagingBufferHandle);
        
        m_allocator.DestroyBuffer(stagingBufferHandle, stagingBufferAllocation);
    }


    void VulkanBuffer::CreateIndexBuffer(){
        VkBufferCreateInfo bufferCreateInfo = BufferCreateInfo(
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_SHARING_MODE_EXCLUSIVE,
            m_sizeInBytes);
        m_allocationHandle = m_allocator.AllocateBuffer(&bufferCreateInfo, VMA_MEMORY_USAGE_GPU_ONLY, &m_bufferHandle);

        VkBufferCreateInfo stagingBufferCreateInfo{};

        stagingBufferCreateInfo = BufferCreateInfo(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                    VK_SHARING_MODE_EXCLUSIVE,
                                                    m_sizeInBytes);
        VkBuffer stagingBufferHandle;

        VmaAllocation stagingBufferAllocation =
            m_allocator.AllocateBuffer(
                &stagingBufferCreateInfo,
                VMA_MEMORY_USAGE_CPU_TO_GPU,
                &stagingBufferHandle);

        void *data;
        data = m_allocator.MapMemory(stagingBufferAllocation);
        memcpy(data, m_actualData, m_sizeInBytes);
        m_allocator.UnmapMemory(stagingBufferAllocation);

        Transfer(stagingBufferHandle);    
        m_allocator.DestroyBuffer(stagingBufferHandle, stagingBufferAllocation);
    }

    void VulkanBuffer::Transfer(VkBuffer& stagingBufferHandle){
        VkCommandPool cmdPool;

        VkCommandPoolCreateInfo cmdPoolCreateInfo{};
        cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cmdPoolCreateInfo.queueFamilyIndex = m_device.GraphicsQueueFamilyIndex();
        cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

        VK_CHECK_RESULT(vkCreateCommandPool(m_device.LogicalDeviceHandle(), &cmdPoolCreateInfo, VK_NULL_HANDLE, &cmdPool));

        VkCommandBuffer cmdBuffer;

        VkCommandBufferAllocateInfo cmdBufferAllocateInfo{};
        cmdBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmdBufferAllocateInfo.commandPool = cmdPool;
        cmdBufferAllocateInfo.commandBufferCount = 1;
        cmdBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        vkAllocateCommandBuffers(m_device.LogicalDeviceHandle(), &cmdBufferAllocateInfo, &cmdBuffer);

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
        VK_CHECK_RESULT(vkCreateFence(m_device.LogicalDeviceHandle(), &fenceCreateInfo, nullptr, &fence));

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmdBuffer;

        VK_CHECK_RESULT(vkQueueSubmit(m_device.GraphicsQueueHandle(), 1, &submitInfo, fence));
        vkWaitForFences(m_device.LogicalDeviceHandle(), 1, &fence, VK_TRUE, UINT64_MAX);

        vkDestroyCommandPool(m_device.LogicalDeviceHandle(), cmdPool, nullptr);
        vkDestroyFence(m_device.LogicalDeviceHandle(), fence, nullptr);
    }
}   
