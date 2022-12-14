#include "Timer.h"
#include "../core/Logger.h"
namespace Pathfinding
{

    Timer::Timer(bool show, std::string function, float time) : m_function(function), m_showMeasuredTime(show), m_time(time)
    {
        m_start = std::chrono::steady_clock::now();
    }
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
        if(m_function.empty()){
        APP_TRACE("Time Measured: {} ms",m_currentTimeMeasured.count());
        }
        else if (m_function == "GPU Pathfinding")
        {
            if (m_time>=0){
                if(m_time<=1.0){
                    APP_TRACE("Time Measured [{}]: {} ms", m_function, m_currentTimeMeasured.count());
                }else{
                    float result = (int)m_time;
                    result = (float)result/((float)5.0);
                    APP_TRACE("Time Measured [{}]: {} ms", m_function, (m_time) - float(result));
                }
            }else{
                APP_TRACE("Time Measured [{}]: {} ms", m_function, m_currentTimeMeasured.count());
            }
        }
        else
        {

        
          APP_TRACE("Time Measured [{}]: {} ms", m_function, m_currentTimeMeasured.count());
        }
        }
    }

    float Timer::GetElapsedMiliseconds()
    {
        m_end = std::chrono::steady_clock::now();
        m_currentTimeMeasured = m_end - m_start;

        return m_currentTimeMeasured.count();
    }
    
    float Timer::GetElapsedSeconds()
    {
        m_end = std::chrono::steady_clock::now();
        m_currentTimeMeasured = m_end - m_start;

        return m_currentTimeMeasured.count()/1000.0f;
    }
}