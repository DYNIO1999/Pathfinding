#ifndef _CORE_TIME_H_
#define _CORE_TIME_H_
#include <chrono>
#include <thread>
#include <iostream>

#define GET_NAME(obj) std::string(#obj)

namespace VulkanPathfinding
{
class Timer
{
public:
    Timer(std::string function);
    Timer();
    ~Timer();
    float GetElapsedMiliseconds();
    float GetElapsedSeconds();

private:
    std::chrono::duration<float> m_currentTimeMeasured;
    std::chrono::time_point<std::chrono::steady_clock> m_start, m_end;
    std::string m_function;
    };    
}
#endif