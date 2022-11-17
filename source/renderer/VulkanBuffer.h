#ifndef _RENDERER_VULKAN_BUFFER_H_
#define _RENDERER_VULKAN_BUFFER_H_
#include "VulkanDevice.h"
#include "VulkanAllocator.h"
#include "../core/Logger.h"
namespace Pathfinding{

class VulkanVertexInputDescriptionBuilder;

enum class VulkanBufferType: uint8_t{
    VERTEX_BUFFER,
    INDEX_BUFFER,
    UNIFORM_BUFFER,
    STORAGE_BUFFER
};

class VulkanVertexInputDescription
{

public:
    VulkanVertexInputDescription()
    {
    }
    ~VulkanVertexInputDescription()
    {
   
    }
    friend class VulkanVertexInputDescriptionBuilder;

    std::vector<VkVertexInputBindingDescription> m_bindings;
    std::vector<VkVertexInputAttributeDescription> m_attributes;

    static VulkanVertexInputDescriptionBuilder Create();
};


class VulkanVertexInputDescriptionBuilder
{
    VulkanVertexInputDescription m_description;

public:
    VulkanVertexInputDescriptionBuilder()
    {
        
    }
    ~VulkanVertexInputDescriptionBuilder(){

    }
    operator VulkanVertexInputDescription() const { return std::move(m_description); }

    VulkanVertexInputDescriptionBuilder &AddBinding(
        uint32_t binding,
        uint32_t strideInBytes,
        VkVertexInputRate inputRate)
    {

        VkVertexInputBindingDescription vertexBinding = {};
        vertexBinding.binding = binding;
        vertexBinding.stride = strideInBytes;
        vertexBinding.inputRate = inputRate;
        m_description.m_bindings.push_back(vertexBinding);
        return *this;
    }

    VulkanVertexInputDescriptionBuilder &AddAttribute(
        uint32_t binding,
        uint32_t location,
        VkFormat format,
        uint32_t offset)
    {

        VkVertexInputAttributeDescription attribute = {};
        attribute.binding = binding;
        attribute.location = location;
        attribute.format = format;
        attribute.offset = offset;

        m_description.m_attributes.push_back(attribute);

        return *this;
    }
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

    VulkanBuffer(
        VulkanDevice &device,
        VulkanAllocator &allocator,
        VulkanBufferType type,
        uint64_t sizeInBytes);

    ~VulkanBuffer();

    //void UpdateData(void* data, uint64_t sizeInBytes);
    //void MapData();


    VkBuffer& BufferHandle(){return m_bufferHandle;}
    
    void MapMemory();
    void UnMapMemory();
    void UpdateMemory(void *newdata);

    VulkanBuffer(const VulkanBuffer &) = delete;
    void operator=(const VulkanBuffer &) = delete;
    
private:
    static VkBufferCreateInfo BufferCreateInfo(VkBufferUsageFlags usage, VkSharingMode sharingMode, uint32_t sizeInBytes);
    

    void CreateVertexBuffer();
    void CreateIndexBuffer();
    
    void CreateUniformBuffer();
    
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