#ifndef _RENDERER_VULKAN_INITIALIZERS_H_
#define _RENDERER_VULKAN_INITIALIZERS_H_

#include "VulkanAllocator.h"
#include "VulkanDevice.h"


namespace Pathfinding{


    namespace VulkanInitializers{

        VkBufferCreateInfo BufferCreateInfo(VkBufferUsageFlags usage, VkSharingMode sharingMode, uint32_t sizeInBytes);
    }
}
#endif