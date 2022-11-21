#ifndef _CAMERA_CAMERA_H_
#define _CAMERA_CAMERA_H_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

//#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#define GLM_ENABLE_EXPERIMENTAL

#include "../input/Input.h"
#include "../core/Logger.h"


namespace Pathfinding{
    class Camera
    {
    public:
        Camera(float width, float height);
        ~Camera();

        glm::mat4 View() const
        {
            return glm::lookAt(m_position, m_position + m_front, m_up);
        }

        glm::mat4 Projection() const
        {
            return m_projection;
        }
    
        void Update(float dt);
    private:
        void UpdateCamera();

        float movementSpeed{15.0f};
        float mouseSensitivity{100.0f};

        float m_near{0.1f};
        float m_far{100.0f};
        float m_aspectRatio;

        glm::vec3 m_orientation = glm::vec3(0.0f, 0.0f, 1.0f);
        glm::vec3 m_position = glm::vec3(0.0f, 15.0f, 10.0f);
        glm::vec3 m_up = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 m_front = glm::vec3(0.0f, -1.0f, -1.0f);
        glm::vec3 m_right;
        glm::vec3 m_worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::mat4 m_projection;
        
        bool state{true};
};
}
#endif

