#include "VulkanInitializers.h"

namespace Pathfinding
{
    namespace VulkanInitializers
    {
        VkBufferCreateInfo BufferCreateInfo(VkBufferUsageFlags usage, VkSharingMode sharingMode, uint32_t sizeInBytes){
            VkBufferCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            info.usage = usage;
            info.size = sizeInBytes;
            info.sharingMode = sharingMode;
            return info;
        }
    }
}

  