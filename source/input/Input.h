#ifndef _INPUT_INPUT_H_
#define _INPUT_INPUT_H_
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <utility>
	

namespace VulkanPathfinding{
class Input
{
public:
    
    static std::pair<int,int> MousePosition();
    static bool MouseButtonPressed(int button);
    static bool KeyPressed(int key);

private:
};
}
#endif