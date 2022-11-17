#ifndef _RENDERER_VULKAN_BUFFER_H_
#define _RENDERER_VULKAN_BUFFER_H_
#include "VulkanDevice.h"
#include "VulkanAllocator.h"
#include "../core/Logger.h"
namespace Pathfinding{

class VulkanBufferBuilder;
enum class VulkanBufferType{
    VERTEX_BUFFER,
    INDEX_BUFFER,
    UNIFORM_BUFFER,
    STORAGE_BUFFER
};
class VulkanBuffer
{
public:
    VulkanBuffer(VulkanDevice &device, VulkanAllocator &allocator);

    VulkanBuffer(VulkanBuffer && other):m_deviceRef(other.m_deviceRef), m_allocatorRef(other.m_allocatorRef){
        APP_ERROR("MOVE CONSTRUCTOR");
        this->m_sizeInBytes = other.m_sizeInBytes;
        APP_ERROR(other.m_sizeInBytes);
        this->m_isMappedMemory =other.m_isMappedMemory;
        this->m_actualData = other.m_actualData;
        //this->m_actualData = nullptr;
    }
    ~VulkanBuffer();
    VulkanBuffer &operator=(VulkanBuffer &&other)
    {
        APP_ERROR("MOVE ASSIGMENT");
        this->m_sizeInBytes = other.m_sizeInBytes;
        APP_ERROR(other.m_sizeInBytes);
        this->m_isMappedMemory = other.m_isMappedMemory;
        this->m_actualData = other.m_actualData;

        return *this;
    }

    static VulkanBufferBuilder&& Create(VulkanDevice& device, VulkanAllocator& allocator);
    

    VulkanBufferType m_bufferType;
    friend class VulkanBufferBuilder;
    void* m_actualData{nullptr};
    uint32_t m_sizeInBytes{0};

private:
    void CreateVertexBuffer();

    VulkanDevice& m_deviceRef;
    VulkanAllocator& m_allocatorRef;

    VkBufferCreateInfo m_bufferCreateInfo;
    
    void* m_mappedData{nullptr}; //only if mapped

    VkBuffer m_bufferHandle{nullptr};
    VmaAllocation m_allocationHandle{nullptr};
    
    bool m_isMappedMemory{false};
};


class VulkanBufferBuilder{
    public:
    VulkanBuffer m_buffer;

    VulkanBufferBuilder(VulkanDevice &device, VulkanAllocator &allocator) : m_buffer(device,allocator)
    {
    }

    VulkanBufferBuilder &operator=(VulkanBufferBuilder &&other)
    {
        APP_ERROR("MOVE ASSIGMENT");
        this->m_buffer = other.m_buffer;
        return *this;
    }
    VulkanBufferBuilder&& Type(VulkanBufferType type){
        m_buffer.m_bufferType = type;
        return std::move(*this);
    }
    VulkanBufferBuilder&& Data(void* data, uint32_t sizeInBytes){

        m_buffer.m_actualData =data;
        m_buffer.m_sizeInBytes = sizeInBytes;

        return std::move(*this);
    }
    VulkanBufferBuilder&& MapMemory(){

        m_buffer.m_isMappedMemory =true;
        return std::move(*this);
    }

    std::shared_ptr<VulkanBuffer> Build(){
        std::shared_ptr<VulkanBuffer> obj;
        obj = std::make_shared<VulkanBuffer>(std::move(this->m_buffer));

        if (obj->m_bufferType == VulkanBufferType::VERTEX_BUFFER)
        {
            APP_ERROR("HEHHE");
            obj->CreateVertexBuffer();
        }
        else if (obj->m_bufferType == VulkanBufferType::INDEX_BUFFER)
        {
        }
        else if (obj->m_bufferType == VulkanBufferType::UNIFORM_BUFFER)
        {
        }
        else
        {
        }
        return obj;
    }
    
    private:
};


}
#endif