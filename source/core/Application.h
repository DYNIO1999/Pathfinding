#ifndef _CORE_APPLICATION_H_
#define _CORE_APPLICATION_H_

//STL
#include <iostream>


//Own
#include "../window/Window.h"
#include "Logger.h"

namespace VulkanPathfinding{
struct FPSCounter{
    void CalculateFPS(float curTime){

        if((curTime - lastFrameTime) > 1.0 || frames == 0){

        }

    }
    void Update(){
        frames++;
    }
    int Fps(){
        return framePerSecond;
    }
    int framePerSecond;
    int frames;
    int limitFramePerSecond{30}; //?
    float lastFrameTime;
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