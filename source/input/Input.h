#ifndef _INPUT_INPUT_H_
#define _INPUT_INPUT_H_
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <utility>
	

namespace Pathfinding{
class Input
{
public:
    static int s_oldKeyStatus;
    static std::pair<int,int> MousePosition();
    static bool MouseButtonPressed(int button);
    static bool KeyPressed(int key);
    static bool KeyPressedOnce(int key);
};
}
#endif