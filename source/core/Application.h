#ifndef _CORE_APPLICATION_H_
#define _CORE_APPLICATION_H_

//STL
#include <iostream>


//Own
#include "../window/Window.h"
#include "Timer.h"
#include "Logger.h"
namespace VulkanPathfinding{
struct FPSCounter{
    void Update(){
        frameCount++;
        if (interval.GetElapsedSeconds() > 1.0f)
        {
            APP_INFO("TIMER: {}", interval.GetElapsedSeconds());
            framePerSecond = frameCount;
            frameCount = 0;
            interval = Timer();
        }
    }
    
    Timer interval;
    int frameCount;
    int framePerSecond;

    float frameElapsedTime;
    float startFrameTime;
    float endFrameTime;
};


struct DeltaTime{

    float deltaTime;
    float currentTime;
    float lastTime;
    void Update(){
        lastTime = currentTime;
    }
    void CalculateDeltaTime(float curTime){
        currentTime = curTime;
        deltaTime = currentTime - lastTime;
    }
    float AsMiliSeconds(){
        return deltaTime*1000.0f;
    }
    float AsSeconds(){
        return deltaTime;
    }
};
class Application
{
public:
    Application(bool enableValidation);
    ~Application();
    void Run();
    std::shared_ptr<Window> GetWindow(){return m_window;}
private:
    static std::shared_ptr<Window> m_window;
    
    void Initialize();
    void Shutdown();

    void InitVulkan();
    
    VkResult CreateVulkanInstance(bool enableValidation);

    bool m_enableValidation;
    bool m_isRunning;

    FPSCounter m_fpsCounter{};
    DeltaTime m_deltaTime{};

};
}
#endif