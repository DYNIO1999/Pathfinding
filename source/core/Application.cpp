#include "Application.h"
#include "../input/Input.h"
namespace Pathfinding{

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
        InitObjects();
      


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


        std::filesystem::path cwd = std::filesystem::current_path();
        std::cout<<cwd<<'\n';
        
        Timer timer(true,GET_NAME(Initialize()));
        
        Logger::Init();

        m_window = std::make_shared<Window>(1600,900,std::move("Pathfinding"));
        m_context = std::make_unique<VulkanContext>();

        m_device = std::make_unique<VulkanDevice>();
        m_allocator = std::make_unique<VulkanAllocator>(*m_device);

        m_swapchain = std::make_unique<VulkanSwapChain>(*m_device);

        m_defaultPipelineSpec = VulkanPipeline::DefaultPipelineSpecification(m_swapchain->RenderPassHandle());
        m_defaultPipelineSpec.vertexInputDescription = VertexInputDescription::GetVertexDescription();
        m_defaultPipelineSpec.pushConstantData = std::make_unique<PipelinePushConstantData>(PipelinePushConstantData{glm::mat4(1.0f)});

        m_defaultPipline = std::make_unique<VulkanPipeline>(*m_device, "../shaders/test.vert.spv", "../shaders/test.frag.spv", m_defaultPipelineSpec);
        

        CreateCommandBuffers();
    }
    void Application::Shutdown(){

        m_allocator->DestroyBuffer(vertexBuffer,vertexBufferAllocation);
        vkDeviceWaitIdle(m_device->LogicalDeviceHandle());
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
        }
    }
    void Application::Draw(){
        uint32_t imageIndex;
        VkResult acquireResult = m_swapchain->AcquireNextImage(&imageIndex);
        if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR){
            vkDeviceWaitIdle(m_device->LogicalDeviceHandle());

            if (m_swapchain == nullptr)
            {
                m_swapchain = std::make_unique<VulkanSwapChain>(*m_device);
            }
            else
            {
                std::shared_ptr<VulkanSwapChain> oldSwapChain = std::move(m_swapchain);
                m_swapchain = std::make_unique<VulkanSwapChain>(*m_device, oldSwapChain);
            }
        }

        if ((acquireResult != VK_SUCCESS && acquireResult != VK_SUBOPTIMAL_KHR))
        {
            APP_ERROR("Failed to acquire swap chain image!");
        }


        VkCommandBuffer commandBuffer = commandBuffers[m_swapchain->CurrentFrame()];
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);
        
        
        // Render Pass
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_swapchain->RenderPassHandle();
        renderPassInfo.framebuffer = m_swapchain->AcquireFrameBuffer(imageIndex);

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = m_swapchain->Extent();
        VkClearValue clearColor;

        
        if(Input::KeyPressed(GLFW_KEY_W)){
            clearColor = {{{0.0f, 1.0f, 0.5f, 1.0f}}};
        }else{
            clearColor = {{{0.6f, 1.0f, 0.5f, 1.0f}}};
        }

        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;


        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_swapchain->Extent().width);
        viewport.height = static_cast<float>(m_swapchain->Extent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, m_swapchain->Extent()};
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        m_defaultPipline->Bind(commandBuffer);


        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer_2, &offset);



        // calculate final mesh matrix
        view= glm::translate(glm::mat4(1.f), camPos);
        model = glm::rotate(model,glm::radians(1.0f), glm::vec3(0.0f,0.0f,1.0f));
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -m_deltaTime.AsSeconds()));

        glm::mat4 mesh_matrix = projection * view * model;
        PipelinePushConstantData constant;
        constant.projection = mesh_matrix;


        // upload the matrix to the GPU via push constants
        vkCmdPushConstants(commandBuffer, m_defaultPipline->PipelineLayoutHandle(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PipelinePushConstantData), &constant);

        // we can now draw

        if (Input::KeyPressed(GLFW_KEY_W))
        {
            // Draw2();
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, &offset);
            vkCmdDraw(commandBuffer, vertices.size(), 1, 0, 0);
        }
        else
        {

            vkCmdDraw(commandBuffer, vertices_2.size(), 1, 0, 0);
        }


        vkCmdEndRenderPass(commandBuffer);
        // Render Pass



        vkEndCommandBuffer(commandBuffer);


        
        VkResult submitResult= m_swapchain->SubmitCommandBuffers(&commandBuffer, &imageIndex);
        if (submitResult == VK_ERROR_OUT_OF_DATE_KHR || submitResult == VK_SUBOPTIMAL_KHR)
        {
            vkDeviceWaitIdle(m_device->LogicalDeviceHandle());
            if (m_swapchain == nullptr)
            {
                m_swapchain = std::make_unique<VulkanSwapChain>(*m_device);
            }
            else
            {
                std::shared_ptr<VulkanSwapChain> oldSwapChain = std::move(m_swapchain);
                m_swapchain = std::make_unique<VulkanSwapChain>(*m_device, oldSwapChain);
            }
        }

    }

    void Application::InitObjects(){
        vertices.emplace_back(Vertex{glm::vec3(-0.5f,-0.5f,0.0f), glm::vec4(1.0f,1.0f,0.5f,0.0f)});
        vertices.emplace_back(Vertex{glm::vec3(0.5f, -0.5f, 0.0f), glm::vec4(1.0f, 1.0f, 0.5f, 0.0f)});
        vertices.emplace_back(Vertex{glm::vec3(0.5f, 0.5f, 0.0f), glm::vec4(1.0f, 1.0f, 0.5f, 0.0f)});
        vertices.emplace_back(Vertex{glm::vec3(0.5f, 0.5f, 0.0f), glm::vec4(1.0f, 1.0f, 0.5f, 0.0f)});
        vertices.emplace_back(Vertex{glm::vec3(-0.5f, 0.5f, 0.0f), glm::vec4(1.0f, 1.0f, 0.5f, 0.0f)});
        vertices.emplace_back(Vertex{glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec4(1.0f, 1.0f, 0.5f, 0.0f)});

        for (auto &i : vertices)
        {
            APP_WARN("Position {}", glm::to_string(i.position));
            APP_WARN("Color {}", glm::to_string(i.color));
        }

        vertices_2.emplace_back(Vertex{glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec4(1.0f, 1.0f, 0.5f, 0.0f)});
        vertices_2.emplace_back(Vertex{glm::vec3(0.5f, -0.5f, 0.0f), glm::vec4(1.0f, 1.0f, 0.5f, 0.0f)});
        vertices_2.emplace_back(Vertex{glm::vec3(0.5f, 0.5f, 0.0f), glm::vec4(1.0f, 1.0f, 0.5f, 0.0f)});


        CreateVertexBuffer();

    }

    void Application::CreateVertexBuffer(){
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = vertices.size() * sizeof(Vertex);
        bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

        vertexBufferAllocation = m_allocator->AllocateBuffer(&bufferInfo, &vertexBuffer);
        auto data = m_allocator->MapMemory(vertexBufferAllocation);

        memcpy(data, vertices.data(), vertices.size() * sizeof(Vertex));
        m_allocator->UnmapMemory(vertexBufferAllocation);

        VkBufferCreateInfo bufferInfo_2 = {};
        bufferInfo_2.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo_2.size = vertices_2.size() * sizeof(Vertex);
        bufferInfo_2.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

        vertexBufferAllocation_2 = m_allocator->AllocateBuffer(&bufferInfo_2, &vertexBuffer_2);
        auto data2 = m_allocator->MapMemory(vertexBufferAllocation_2);

        memcpy(data2, vertices_2.data(), vertices_2.size() * sizeof(Vertex));
        m_allocator->UnmapMemory(vertexBufferAllocation_2);
    }

}

