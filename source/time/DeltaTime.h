#ifndef _TIME_DELTA_TIME_H_
#define _TIME_DELTA_TIME_H_

namespace Pathfinding{

    class DeltaTime
    {
    public:
        DeltaTime();
        ~DeltaTime();
        void Update(float currentTime);

        float AsMiliSeconds()
        {
            return m_deltaTime * 1000.0f;
        }
        float AsSeconds()
        {
            return m_deltaTime;
        }

    private:
        float m_deltaTime;
        float m_currentTime;
        float m_lastTime;
};
}
#endif