#ifndef _CAMERA_CAMERA_H_
#define _CAMERA_CAMERA_H_

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
//#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#define GLM_ENABLE_EXPERIMENTAL

#include "../input/Input.h"

namespace Pathfinding{
class Camera
{
public:
    Camera(float width , float height):projection(glm::mat4(1.0f)), view(glm::mat4(1.0f)), m_cameraSpeed(5.0f){

        m_aspectRatio = width/height;
        projection = glm::perspective(glm::radians(60.0f), m_aspectRatio, m_near, m_far);
        glm::vec3 camPos = {0.f, 0.0f, -10.0f};
        //glm::mat4 view = glm::translate(glm::mat4(1.f), camPos);
        view = glm::translate(view, camPos);
    }
    ~Camera(){

    }
    //glm::perspective(glm::radians(70.f), 1600.f / 900.f, 0.1f, 200.0f);
    
    glm::mat4 projection;
    glm::mat4 view;
    float m_cameraSpeed;
    float m_aspectRatio;
    float m_near{0.1f};
    float m_far{200.0f};

    void Update(float dt){
        if(Input::KeyPressed(GLFW_KEY_W)){
            view = glm::translate(view, glm::vec3(0.0f,0.0f,m_cameraSpeed*dt));
        }
        if(Input::KeyPressed(GLFW_KEY_S)){
            view = glm::translate(view, glm::vec3(0.0f,0.0f,-m_cameraSpeed*dt));
        }

        if(Input::KeyPressed(GLFW_KEY_A)){
            view = glm::translate(view, glm::vec3(m_cameraSpeed*dt, 0.0f, 0.0f));
        }

        if (Input::KeyPressed(GLFW_KEY_D))
        {
            view = glm::translate(view, glm::vec3(-m_cameraSpeed* dt, 0.0f, 0.0f));
        }
    }

private:

};
}
#endif
