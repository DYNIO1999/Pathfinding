#include "FPSCounter.h"

namespace VulkanPathfinding
{
        FPSCounter::FPSCounter():
        m_frameCount(0),
        m_framePerSecond(0),
        m_interval(false)
        {

        }
        FPSCounter::~FPSCounter(){

        }
        void FPSCounter::Update(){
            m_frameCount++;
            if (m_interval.GetElapsedSeconds() > 1.0f)
            {
                m_framePerSecond = m_frameCount;
                m_frameCount = 0;
                m_interval = Timer(false);
            }
        }
} 
