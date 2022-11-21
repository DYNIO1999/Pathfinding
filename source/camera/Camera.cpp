#include "Camera.h"
#include "../core/Application.h"

namespace Pathfinding
{

    Camera::Camera(float width, float height)
    {
        m_aspectRatio =width/height;
        m_projection = glm::perspective(glm::radians(45.0f), m_aspectRatio, m_near, m_far);
        UpdateCamera();
    }
    Camera::~Camera()
    {
    }
  void Camera::Update(float dt){

      float velocity = movementSpeed * dt;
      if (Input::KeyPressed(GLFW_KEY_W))
      {
          if (state){
              m_position -= (glm::vec3(0.0f, 0.0f, 1.0f) * velocity);
          }
          else{
              m_position += m_front * velocity;
          }
      }
      if (Input::KeyPressed(GLFW_KEY_S))
      {
        if(state){
            m_position += (glm::vec3(0.0f,0.0f,1.0f)* velocity);
        }
        else{  
            m_position -= m_front * velocity;
        }
      }

      if (Input::KeyPressed(GLFW_KEY_A))
      {
          m_position -= m_right * velocity;
      }

      if (Input::KeyPressed(GLFW_KEY_D))
      {
          m_position += m_right * velocity;
      }

      auto [x, y] = Input::MousePosition();
      float xx = static_cast<float>(x);
      float yy = static_cast<float>(y);

      auto [width,height] =  Application::GetWindow()->WindowSize();
      
      float rotX = (float)(xx - (static_cast<float>(width/ 2.0f))) /
                   static_cast<float>(width);
      float rotY = (float)(yy - (static_cast<float>(height / 2.0f))) /
                   static_cast<float>(height);

      rotX = rotX *mouseSensitivity;
      rotY = rotY * mouseSensitivity;

     auto result = Input::KeyPressedOnce(GLFW_KEY_ESCAPE);
     if(result){
        state = !state;
     }
    if(!state){
      glm::vec3 newOrientation = glm::rotate(m_front, glm::radians((-rotY)), glm::normalize(glm::cross(m_front, m_up)));
      if (std::abs(glm::angle(newOrientation, m_up) - glm::radians(90.0f)) <= glm::radians(85.0f))
      {
          m_front = newOrientation;
      }
      m_front = glm::rotate(m_front, glm::radians((-rotX)), m_up);
      UpdateCamera();
      glfwSetCursorPos(Application::GetWindow()->GetWindowHandle(), (static_cast<float>(width / 2.0f)), (static_cast<float>(height / 2.0f)));
    }
  }

  void Camera::UpdateCamera(){
      m_right = glm::normalize(glm::cross(m_front, m_worldUp));
      m_up = glm::normalize(glm::cross(m_right, m_front));
  }
} 
