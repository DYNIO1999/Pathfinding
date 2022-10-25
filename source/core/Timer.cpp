#include "Timer.h"
namespace VulkanPathfinding
{
    Timer::Timer()
    {
        m_start = std::chrono::system_clock::now();
        
    }
    
    Timer::~Timer()
    {
        m_end = std::chrono::system_clock::now();
        m_currentTimeMeasured = m_end-m_start;
        std::cout<<"TIME MEASURED: "<<m_currentTimeMeasured.count()*1000<<" ms"<<'\n';
    }

    float Timer::GetElapsedMiliseconds()
    {
        m_end = std::chrono::system_clock::now();
        m_currentTimeMeasured = m_end - m_start;

        return m_currentTimeMeasured.count()*1000;
    }
    
    float Timer::GetElapsedSeconds()
    {
        m_end = std::chrono::system_clock::now();
        m_currentTimeMeasured = m_end - m_start;

        return m_currentTimeMeasured.count();
    }
}