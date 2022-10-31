#include "DeltaTime.h"


namespace VulkanPathfinding{

    DeltaTime::DeltaTime():
    m_deltaTime(0.0f), 
    m_currentTime(0.0f), 
    m_lastTime(0.0f)
    {

    }

    DeltaTime::~DeltaTime()
    {

    }

    void DeltaTime::Update(float currentTime)
    {
        m_currentTime = currentTime;
        m_deltaTime = m_currentTime - m_lastTime;
        m_lastTime =m_currentTime;
    }
}