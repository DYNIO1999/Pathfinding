#ifndef _RENDERER_VULKAN_BUFFERS_H_
#define _RENDERER_VULKAN_BUFFERS_H_

#include "VulkanAllocator.h"
#include "VulkanDevice.h"

#include "../objects/Object.h"
namespace Pathfinding{

    struct VertexInputDescription
    {

        std::vector<VkVertexInputBindingDescription> bindings;
        std::vector<VkVertexInputAttributeDescription> attributes;

        static VertexInputDescription GetVertexDescription()
        {
            VertexInputDescription description;

            VkVertexInputBindingDescription mainBinding = {};
            mainBinding.binding = 0;
            mainBinding.stride = sizeof(Vertex);
            mainBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            description.bindings.push_back(mainBinding);

            VkVertexInputAttributeDescription positionAttribute = {};
            positionAttribute.binding = 0;
            positionAttribute.location = 0;
            positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
            positionAttribute.offset = offsetof(Vertex, position);

            VkVertexInputAttributeDescription colorAttribute = {};
            colorAttribute.binding = 0;
            colorAttribute.location = 1;
            colorAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
            colorAttribute.offset = offsetof(Vertex, color);

            description.attributes.push_back(positionAttribute);
            description.attributes.push_back(colorAttribute);
            return description;
        }
    };

class VulkanVertexBuffer
{
public:
    VulkanVertexBuffer(VulkanDevice& device, VulkanAllocator& allocator, void *data, uint32_t sizeInBytes);
    ~VulkanVertexBuffer();

    VkBuffer* BufferHandle(){return &m_bufferHandle;}
private:
    VulkanDevice& m_deviceRef;
    VulkanAllocator& m_allocatorRef;
    VkBuffer m_bufferHandle;
    VmaAllocation m_allocationHandle; 

    void Create(void *data, uint32_t sizeInBytes);
};

class VulkanIndexBuffer
{
public:
    VulkanIndexBuffer(VulkanDevice &device, VulkanAllocator &allocator, void *data, uint32_t sizeInBytes);
    ~VulkanIndexBuffer();

    uint32_t IndexCount(){return m_indicesCount;}
    VkBuffer BufferHandle() { return m_bufferHandle; }
private:
    uint32_t m_indicesCount;
    VulkanDevice &m_deviceRef;
    VulkanAllocator &m_allocatorRef;
    VkBuffer m_bufferHandle;
    VmaAllocation m_allocationHandle;

    void Create(void *data, uint32_t sizeInBytes);

};
}

#endif
