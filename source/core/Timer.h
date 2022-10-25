#ifndef _CORE_TIME_H_
#define _CORE_TIME_H_
#include <chrono>
#include <thread>
#include <iostream>
namespace VulkanPathfinding
{


class Timer
{
public:

    Timer();
    ~Timer();
    float GetElapsedMiliseconds();
    float GetElapsedSeconds();

private:
    std::chrono::duration<float> m_currentTimeMeasured;
    std::chrono::time_point<std::chrono::system_clock> m_start, m_end;
    };    
}
#endif