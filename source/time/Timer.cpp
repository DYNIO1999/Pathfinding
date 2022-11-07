#include "Timer.h"
#include "../core/Logger.h"
namespace Pathfinding
{

    Timer::Timer(bool show, std::string function) : m_function(function), m_showMeasuredTime(show)
    {
        m_start = std::chrono::steady_clock::now();
    }

    Timer::Timer(bool show) : m_showMeasuredTime(show)
    {
        m_start = std::chrono::steady_clock::now();
        
    }
    
    Timer::~Timer()
    {
        m_end = std::chrono::steady_clock::now();
        m_currentTimeMeasured = m_end-m_start;

        if(m_showMeasuredTime){
        if(m_function.empty())
        APP_TRACE("Time Measured: {} ms",m_currentTimeMeasured.count()*1000);
        else
        APP_TRACE("Time Measured [{}]: {} ms",m_function,m_currentTimeMeasured.count()*1000);
        }
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