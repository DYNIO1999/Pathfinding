#ifndef _TIME_TIMER_H_
#define _TIME_TIMER_H_
#include <chrono>
#include <thread>
#include <string>

#define GET_NAME(obj) std::string(#obj)

namespace Pathfinding
{
class Timer
{
public:
    Timer(bool show, std::string function);
    Timer(bool show);
    ~Timer();
    float GetElapsedMiliseconds();
    float GetElapsedSeconds();

private:
    std::chrono::duration<float, std::milli> m_currentTimeMeasured;
    std::chrono::time_point<std::chrono::steady_clock> m_start, m_end;
    std::string m_function;
    bool m_showMeasuredTime;
    };    
}
#endif