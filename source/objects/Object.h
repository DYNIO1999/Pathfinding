#ifndef _OBJECTS_OBJECT_H_
#define _OBJECTS_OBJECT_H_

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
//#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#define GLM_ENABLE_EXPERIMENTAL


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Pathfinding{

    struct Vertex{
        glm::vec3 position;
        glm::vec4 color;
    };


    struct UniformBuffer
    {
        glm::mat4 modelMatrix;
    };
}
#endif