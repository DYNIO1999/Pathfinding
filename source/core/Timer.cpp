#include "Timer.h"
#include "Logger.h"
namespace VulkanPathfinding
{
    Timer::Timer(std::string function):m_function(function)
    {
        m_start = std::chrono::steady_clock::now();
    }


    Timer::Timer()
    {
        m_start = std::chrono::steady_clock::now();
        
    }
    
    Timer::~Timer()
    {
        m_end = std::chrono::steady_clock::now();
        m_currentTimeMeasured = m_end-m_start;

        if(m_function.empty())
        APP_TRACE("Time Measured: {} ms",m_currentTimeMeasured.count()*1000);
        else
        APP_TRACE("Time Measured [{}]: {} ms",m_function,m_currentTimeMeasured.count()*1000);
    }

    float Timer::GetElapsedMiliseconds()
    {
        m_end = std::chrono::steady_clock::now();
        m_currentTimeMeasured = m_end - m_start;

        return m_currentTimeMeasured.count()*1000;
    }
    
    float Timer::GetElapsedSeconds()
    {
        m_end = std::chrono::steady_clock::now();
        m_currentTimeMeasured = m_end - m_start;

        return m_currentTimeMeasured.count();
    }
}