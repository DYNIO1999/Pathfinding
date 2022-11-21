#ifndef _INPUT_INPUT_H_
#define _INPUT_INPUT_H_
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <utility>
#include <unordered_map>	

namespace Pathfinding{

struct KeyStatus {
    int keyPressed;
};

class Input
{
public:
    static int s_oldKeyStatus;
    static int s_currentKey;
    static std::pair<int,int> MousePosition();
    static bool MouseButtonPressed(int button);
    static bool KeyPressed(int key);
    static bool KeyPressedOnce(int key);



    static std::unordered_map<int, KeyStatus> s_KeyStatus;

};
}
#endif