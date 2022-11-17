#ifndef _RENDERER_VULKAN_BUFFER_H_
#define _RENDERER_VULKAN_BUFFER_H_
#include "VulkanDevice.h"
#include "VulkanAllocator.h"
#include "../core/Logger.h"
namespace Pathfinding{

enum class VulkanBufferType: uint8_t{
    VERTEX_BUFFER,
    INDEX_BUFFER,
    UNIFORM_BUFFER,
    STORAGE_BUFFER
};

class VulkanBuffer
{
public:
    VulkanBuffer(
    VulkanDevice& device, 
    VulkanAllocator& allocator,
    VulkanBufferType type,
    void* data, 
    uint64_t sizeInBytes);
    
    ~VulkanBuffer();

    //void UpdateData(void* data, uint64_t sizeInBytes);
    //void MapData();


    VkBuffer& BufferHandle(){return m_bufferHandle;}

    VulkanBuffer(const VulkanBuffer &) = delete;
    void operator=(const VulkanBuffer &) = delete;
    
private:
    static VkBufferCreateInfo BufferCreateInfo(VkBufferUsageFlags usage, VkSharingMode sharingMode, uint32_t sizeInBytes);
    

    
    void CreateVertexBuffer();
    void CreateIndexBuffer();
    void Transfer(VkBuffer &stagingBufferHandle);

    VulkanDevice& m_device;
    VulkanAllocator& m_allocator;
    VulkanBufferType m_type;
    void* m_actualData;
    uint64_t m_sizeInBytes;
    
    
    VkBuffer m_bufferHandle{nullptr};
    VmaAllocation m_allocationHandle{nullptr};
    void* m_mappedData{nullptr};
};
}
#endif