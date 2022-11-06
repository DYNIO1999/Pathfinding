#ifndef _OBJECTS_OBJECT_H_
#define _OBJECTS_OBJECT_H_

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
//#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#define GLM_ENABLE_EXPERIMENTAL


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace VulkanPathfinding{

    struct Vertex{
        glm::vec3 position;
        glm::vec4 color;
    };

    struct VertexInputDescription
    {

        std::vector<VkVertexInputBindingDescription> bindings;
        std::vector<VkVertexInputAttributeDescription> attributes;

        static VertexInputDescription GetVertexDescription(){
            VertexInputDescription description;

            // we will have just 1 vertex buffer binding, with a per-vertex rate
            VkVertexInputBindingDescription mainBinding = {};
            mainBinding.binding = 0;
            mainBinding.stride = sizeof(Vertex);
            mainBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            description.bindings.push_back(mainBinding);

            // Position will be stored at Location 0
            VkVertexInputAttributeDescription positionAttribute = {};
            positionAttribute.binding = 0;
            positionAttribute.location = 0;
            positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
            positionAttribute.offset = offsetof(Vertex, position);

            // Color will be stored at Location 1
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
}
#endif