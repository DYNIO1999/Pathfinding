#include "Application.h"
#include "../input/Input.h"
namespace VulkanPathfinding{

    std::shared_ptr<Window> Application::m_window;
    bool Application::m_enableValidation = false;

    Application::Application(bool enableValidation)
    {
        m_enableValidation = enableValidation;
    
    }
    Application::~Application(){

    }

    void Application::Run(){
        //Timer timer(true);
        Initialize();



        //Init Vulkan
        while (m_window->IsOpen())
        {

            //Delta Time
            m_deltaTime.Update(static_cast<float>(glfwGetTime()));
            //APP_TRACE("Delta Time: {}", m_deltaTime.AsMiliSeconds());
            //Delta Time
            
            //FRAME START
            //dooo update
            //doo render

            //auto [x,y] = Input::MousePosition();
            //APP_INFO("MOUSE_POSITION  X:{} Y:{}",x,y);
            //FRAME END
            Draw();
            m_window->ProcessEvents(); //process events/inputs                        
            
            m_fpsCounter.Update();
            
            //if(m_fpsCounter.GetFPS()>0)
            //    APP_TRACE("FPS {}", m_fpsCounter.GetFPS());
            
        }
        Shutdown();

    } 


    void Application::Initialize(){
        Timer timer(true,GET_NAME(Initialize()));
        
        Logger::Init();

        m_window = std::make_shared<Window>(1600,900,std::move("VulkanPathfinding"));

        auto &vulkanContext = VulkanContext::Get();
        
        vulkanContext.Initialize();
        m_device = std::make_unique<VulkanDevice>();
        m_device->Initialize();

        m_swapchain = std::make_unique<VulkanSwapChain>(*m_device,nullptr);


        

        VulkanPipeline::DefaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass =m_swapchain->m_renderPass;
        m_defaultPipline = std::make_unique<VulkanPipeline>(*m_device, "../shaders/test.vert.spv", "../shaders/test.frag.spv",pipelineConfig);

        CreateCommandBuffers();
    }
    void Application::Shutdown(){
        vkDeviceWaitIdle(m_device->LogicalDeviceHandle());
        
        m_swapchain->Destroy();
        m_device->Shutdown();
        auto &vulkanContext = VulkanContext::Get();
        vulkanContext.Shutdown();

    }

    void Application::CreateCommandBuffers(){
        commandBuffers.resize(2);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_device->CommandPoolHandle();
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        if (vkAllocateCommandBuffers(m_device->LogicalDeviceHandle(), &allocInfo, commandBuffers.data()) !=
            VK_SUCCESS)
        {
            APP_ERROR("DOESNT WORK");
            //throw std::runtime_error("failed to allocate command buffers!");
        }
    }
    void Application::Draw(){
        uint32_t imageIndex;
       
        m_swapchain->AcquireNextImage(&imageIndex);



        //isFrameStarted = true;

        auto commandBuffer = commandBuffers[m_swapchain->m_currentFrame];
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        {
            APP_ERROR("DOESNT WORK");
        }


        
        
        
        
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_swapchain->m_renderPass;
        renderPassInfo.framebuffer = m_swapchain->m_swapChainFramebuffers[imageIndex];

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = m_swapchain->m_swapChainExtent;

        VkClearValue clearColor = {{{1.0f, 1.0f, 0.5f, 1.0f}}};

        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_swapchain->m_swapChainExtent.width);
        viewport.height = static_cast<float>(m_swapchain->m_swapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, m_swapchain->m_swapChainExtent};
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        m_defaultPipline->Bind(commandBuffer);
        
        vkCmdDraw(commandBuffer, 3, 1, 0, 0);

        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        {
            APP_ERROR("DOESNT WORK");
        }

        m_swapchain->SubmitCommandBuffers(&commandBuffer, &imageIndex);
    }

}

