#ifndef _TIME_FPS_COUNTER_H_
#define _TIME_FPS_COUNTER_H_
#include "Timer.h"
namespace Pathfinding{
    
    class FPSCounter
    {
    public:
        FPSCounter();
        ~FPSCounter();
        void Update();
        int GetFPS() const{
            return m_framePerSecond;
        }
    private:
        int m_frameCount;
        int m_framePerSecond;
        Timer m_interval;
};
}
#endif