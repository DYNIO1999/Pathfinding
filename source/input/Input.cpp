#include "Input.h"
#include "../core/Logger.h"
#include "../core/Application.h"
namespace Pathfinding
{
    std::unordered_map<int, KeyStatus> Input::s_KeyStatus;

    std::pair<int, int> Input::MousePosition(){
        double xpos, ypos;
        auto window = Application::GetWindow();
        glfwGetCursorPos(window->GetWindowHandle(), &xpos, &ypos);
        return {static_cast<int>(xpos), static_cast<int>(ypos)};
    }
    bool Input::MouseButtonPressed(int button)
    {
        auto window = Application::GetWindow();
        int status = glfwGetMouseButton(window->GetWindowHandle(), button);
        return status == GLFW_PRESS;

    }
    bool Input::KeyPressed(int key)
    {
        auto window = Application::GetWindow();
        int status = glfwGetKey(window->GetWindowHandle(), key);
        return status == GLFW_PRESS || status == GLFW_REPEAT;
    }
    int Input::s_oldKeyStatus = GLFW_RELEASE;
    int Input::s_currentKey=0;
    bool Input::KeyPressedOnce(int key){
        bool state =false;
        auto window = Application::GetWindow();
        
        int status = glfwGetKey(window->GetWindowHandle(), key);
        if(s_KeyStatus.find(key) !=s_KeyStatus.end()){
            auto it = s_KeyStatus.find(key);
            s_oldKeyStatus = it->second.keyPressed;
            APP_ERROR("CHECK");
        }
        if( status== GLFW_RELEASE && s_oldKeyStatus ==GLFW_PRESS){
            state = true;
        }
        if(s_KeyStatus.find(key) !=s_KeyStatus.end()){
            auto it = s_KeyStatus.find(key);
            it->second.keyPressed = status;
        }else{
            s_KeyStatus.insert(std::pair <int,KeyStatus>(status,KeyStatus{status}));
        }

        return state;
    }
}
