#include "Application.h"
#include "../input/Input.h"
namespace Pathfinding
{

    std::shared_ptr<Window> Application::m_window;
    bool Application::m_enableValidation = false;

    Application::Application(bool enableValidation)
    {
        m_enableValidation = enableValidation;
    }
    Application::~Application()
    {
    }

    void Application::Run()
    {
        Initialize();

        CreateCommandBuffers(); 

        CreateDescriptorPool();
        CreateDescriptorSets();
        

        CreateComputePipelineLayout();
        CreateComputePipeline("../shaders/test.comp.spv");
        
        CreateComputeStorageBuffers();
        CreateComputeDescriptorPool();
        CreateComputeCommandPool();

        BuildComputeCommands();

        
        while (m_window->IsOpen())
        {

            m_deltaTime.Update(static_cast<float>(glfwGetTime()));
                        
            Update();
            Draw();
            m_window->ProcessEvents(); 

            m_fpsCounter.Update();
            if(Input::KeyPressedOnce(GLFW_KEY_F)){
                if(m_fpsCounter.GetFPS()>0)
                    APP_TRACE("FPS {}", m_fpsCounter.GetFPS());
            }

            if (Input::KeyPressedOnce(GLFW_KEY_M))
                m_allocator->ShowAllocatedMemory();
            if(Input::KeyPressedOnce(GLFW_KEY_N)){
                if (m_statsObjectsRendered)
                    APP_TRACE("Number of objects currently being rendered: {}", m_statsObjectsRendered);
            }

        }
        Shutdown();
    }

    void Application::Initialize()
    {
        
        Logger::Init();

        m_window = std::make_shared<Window>(1600, 900, std::move("Pathfinding"));
        m_context = std::make_unique<VulkanContext>();

        m_device = std::make_unique<VulkanDevice>();
        m_allocator = std::make_unique<VulkanAllocator>(*m_device);

        m_swapchain = std::make_unique<VulkanSwapChain>(*m_device, *m_allocator);

        m_defaultPipelineSpec = VulkanPipeline::DefaultPipelineSpecification(m_swapchain->RenderPassHandle());

        VulkanVertexInputDescription vertexDescription = VulkanVertexInputDescription::Create().AddBinding(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX).AddAttribute(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)).AddAttribute(0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color));

        m_defaultPipelineSpec.vertexInputDescription = vertexDescription;
        m_defaultPipelineSpec.pushConstantData = std::make_unique<PipelinePushConstantData>(PipelinePushConstantData{glm::vec4(1.0f, 1.0f, 0.5f, 0.0f)});
        m_defaultPipelineSpec.AddDescriptorSetLayout(*m_device);
        m_defaultPipline = std::make_unique<VulkanPipeline>(*m_device, "../shaders/test.vert.spv", "../shaders/test.frag.spv", m_defaultPipelineSpec);

        auto [width, height] = Application::GetWindow()->WindowSize();
        m_camera = std::make_unique<Camera>(static_cast<float>(width), static_cast<float>(height));

        InitGrids();
        InitObjects(); 


        m_vertexBuffer = std::make_unique<VulkanBuffer>(*m_device, *m_allocator, VulkanBufferType::VERTEX_BUFFER, m_vertices.data(), sizeof(Vertex) * m_vertices.size());
        m_indexBuffer = std::make_unique<VulkanBuffer>(*m_device, *m_allocator, VulkanBufferType::INDEX_BUFFER, m_indices.data(), sizeof(u_int32_t) * m_indices.size());

        for (auto i = 0; i < m_swapchain->MAX_FRAMES_IN_FLIGHT; i++)
        {

            m_cameraData.cameraUBOs[i] = std::make_unique<VulkanBuffer>(*m_device, *m_allocator, VulkanBufferType::UNIFORM_BUFFER, sizeof(CameraUBO::Values));
        }

        for (size_t i = 0; i < 2; i++)
        {
            for (size_t j = 0; j < m_gridData.size(); j++)
            {

                m_gridData[j].modelUBOs[i] = std::make_unique<VulkanBuffer>(*m_device, *m_allocator, VulkanBufferType::UNIFORM_BUFFER, sizeof(glm::mat4));
            }
        }

        for (size_t i = 0; i < 2; i++)
        {
            for (size_t j = 0; j < m_obstacles.size(); j++)
            {

                m_obstacles[j].modelUBOs[i] = std::make_unique<VulkanBuffer>(*m_device, *m_allocator, VulkanBufferType::UNIFORM_BUFFER, sizeof(glm::mat4));
            }
        }
        for (size_t i = 0; i < 2; i++)
        {
            for (size_t j = 0; j < m_agents.size(); j++)
            {

                m_agents[j].modelUBOs[i] = std::make_unique<VulkanBuffer>(*m_device, *m_allocator, VulkanBufferType::UNIFORM_BUFFER, sizeof(glm::mat4));
            }
        }
    }
    void Application::Shutdown()
    {

        delete[] m_gridGPU;
        delete[] m_pathGPU;
        
        vkDeviceWaitIdle(m_device->LogicalDeviceHandle());
    
        vkDestroyPipeline(m_device->LogicalDeviceHandle(), m_computePipeline, nullptr);
        vkDestroyPipelineLayout(m_device->LogicalDeviceHandle(), m_pipelineLayoutCompute,nullptr);

        for (size_t i = 0; i < m_computeSetLayouts.size(); i++)
        {
            vkDestroyDescriptorSetLayout(m_device->LogicalDeviceHandle(), m_computeSetLayouts[i], nullptr);
        }
        
        for (int i = 0; i < 2; i++)
        {
            m_allocator->UnmapMemory(m_ssbObjects[i].buffer.allocationHandle);
            m_allocator->DestroyBuffer(m_ssbObjects[i].buffer.bufferHandle, m_ssbObjects[i].buffer.allocationHandle);
        }

        for (auto it : m_defaultPipelineSpec.descriptorSetLayouts)
        {
            vkDestroyDescriptorSetLayout(m_device->LogicalDeviceHandle(), it, nullptr);
        }
        vkDestroyDescriptorPool(m_device->LogicalDeviceHandle(), m_descriptorPoolCompute, nullptr);
        vkDestroyDescriptorPool(m_device->LogicalDeviceHandle(), m_descriptorPool, nullptr);
    }

    void Application::CreateCommandBuffers()
    {
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

    void Application::Update()
    {

        if (Input::KeyPressedOnce(GLFW_KEY_P))
        {
            computePathStart = true;
        }

        if (Input::KeyPressedOnce(GLFW_KEY_O))
        {
            cpuPathStart = true;
        }
        if (computePathStart && !computePathEnd && cpuPathEnd)
        {
            CalculateCompute();
            computePathEnd = true;
        }
        if (cpuPathStart && !cpuPathEnd)
        {
            ResolvePath();
            cpuPathEnd = true;
        }

        m_camera->Update(m_deltaTime.AsSeconds());

        CameraUBO::Values cameraUboValues{};
        cameraUboValues.proj = m_camera->Projection();
        cameraUboValues.view = m_camera->View();
        m_cameraData.cameraUBOs[m_swapchain->CurrentFrame()]->UpdateMemory(&cameraUboValues);

        for (size_t i = 0; i < m_gridData.size(); i++)
        {
            m_gridData[i].modelUBOs[m_swapchain->CurrentFrame()]->UpdateMemory(&m_gridData[i].transform);
        }

        for (size_t i = 0; i < m_obstacles.size(); i++)
        {
            m_obstacles[i].modelUBOs[m_swapchain->CurrentFrame()]->UpdateMemory(&m_obstacles[i].transform);
        }

        if (Input::KeyPressedOnce(GLFW_KEY_SPACE))
        {
            pressed = !pressed;
        }

        if (pressed && (computePathStart || cpuPathStart))
        {
            for (auto &it : m_agents)
            {
                it.Update(m_gridData, m_deltaTime.AsSeconds());
            }
        }
    

        for (size_t i = 0; i < m_agents.size(); i++)
        {
            m_agents[i].modelUBOs[m_swapchain->CurrentFrame()]->UpdateMemory(&m_agents[i].transform);
        }
    }

    void Application::Draw()
    {

        uint32_t imageIndex;
        VkResult acquireResult = m_swapchain->AcquireNextImage(&imageIndex);
        if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR)
        {
            vkDeviceWaitIdle(m_device->LogicalDeviceHandle());

            if (m_swapchain == nullptr)
            {
                m_swapchain = std::make_unique<VulkanSwapChain>(*m_device, *m_allocator);
            }
            else
            {
                std::shared_ptr<VulkanSwapChain> oldSwapChain = std::move(m_swapchain);
                m_swapchain = std::make_unique<VulkanSwapChain>(*m_device, *m_allocator, oldSwapChain);
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

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_swapchain->RenderPassHandle();
        renderPassInfo.framebuffer = m_swapchain->AcquireFrameBuffer(imageIndex);

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = m_swapchain->Extent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {{0.025f, 0.025f, 0.025f, 1.0f}};
        clearValues[1].depthStencil = {1.0f, 0};

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = static_cast<float>(m_swapchain->Extent().height);
        viewport.width = static_cast<float>(m_swapchain->Extent().width);
        viewport.height = -static_cast<float>(m_swapchain->Extent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        
        VkRect2D scissor{{0, 0}, m_swapchain->Extent()};
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        m_defaultPipline->Bind(commandBuffer);

        VkDeviceSize offset = 0;

        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_vertexBuffer->BufferHandle(), &offset);
        vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer->BufferHandle(), 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_defaultPipline->PipelineLayoutHandle(), 0, 1, &m_cameraData.descriptors[m_swapchain->CurrentFrame()], 0, nullptr);
        for (size_t i = 0; i < m_gridData.size(); i++)
        {

            PipelinePushConstantData pushConstant;
            pushConstant.color = m_gridData[i].color;
            for (size_t k = 0; k < NUMBER_OF_AGENTS; k++)
            {
                for (auto &it : m_agents[k].path)
                {
                    if (m_gridData[i].index == it)
                    {
                        pushConstant.color = m_agents[k].color;
                    }
                }
            }
            vkCmdPushConstants(commandBuffer, m_defaultPipline->PipelineLayoutHandle(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PipelinePushConstantData), &pushConstant);
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_defaultPipline->PipelineLayoutHandle(), 1, 1, &m_gridData[i].descriptors[m_swapchain->CurrentFrame()], 0, nullptr);
            vkCmdDrawIndexed(commandBuffer, m_indices.size(), 1, 0, 0, 0);
        }

        for (size_t i = 0; i < m_obstacles.size(); i++)
        {
            PipelinePushConstantData pushConstant;
            pushConstant.color = m_obstacles[i].color;
            vkCmdPushConstants(commandBuffer, m_defaultPipline->PipelineLayoutHandle(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PipelinePushConstantData), &pushConstant);
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_defaultPipline->PipelineLayoutHandle(), 1, 1, &m_obstacles[i].descriptors[m_swapchain->CurrentFrame()], 0, nullptr);
            vkCmdDrawIndexed(commandBuffer, m_indices.size(), 1, 0, 0, 0);
        }

        for (size_t i = 0; i < m_agents.size(); i++)
        {
            PipelinePushConstantData pushConstant;
            pushConstant.color = m_agents[i].color;
            vkCmdPushConstants(commandBuffer, m_defaultPipline->PipelineLayoutHandle(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PipelinePushConstantData), &pushConstant);
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_defaultPipline->PipelineLayoutHandle(), 1, 1, &m_agents[i].descriptors[m_swapchain->CurrentFrame()], 0, nullptr);
            vkCmdDrawIndexed(commandBuffer, m_indices.size(), 1, 0, 0, 0);
        }

        vkCmdEndRenderPass(commandBuffer);
        vkEndCommandBuffer(commandBuffer);

        VkResult submitResult = m_swapchain->SubmitCommandBuffers(&commandBuffer, &imageIndex);
        if (submitResult == VK_ERROR_OUT_OF_DATE_KHR || submitResult == VK_SUBOPTIMAL_KHR)
        {
            vkDeviceWaitIdle(m_device->LogicalDeviceHandle());
            if (m_swapchain == nullptr)
            {
                m_swapchain = std::make_unique<VulkanSwapChain>(*m_device, *m_allocator);
            }
            else
            {
                std::shared_ptr<VulkanSwapChain> oldSwapChain = std::move(m_swapchain);
                m_swapchain = std::make_unique<VulkanSwapChain>(*m_device, *m_allocator, oldSwapChain);
            }
        }
    }

    void Application::InitObjects()
    {

        m_vertices.emplace_back(Vertex{glm::vec3(-1.0f, 1.0f, -1.0f), glm::vec4(1.0f, 1.0f, 0.5f, 0.0f)});
        m_vertices.emplace_back(Vertex{glm::vec3(1.0f, 1.0f, -1.0f), glm::vec4(1.0f, 1.0f, 0.5f, 0.0f)});
        m_vertices.emplace_back(Vertex{glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec4(1.0f, 1.0f, 0.5f, 0.0f)});
        m_vertices.emplace_back(Vertex{glm::vec3(1.0f, -1.0f, -1.0f), glm::vec4(1.0f, 1.0f, 0.5f, 0.0f)});
        m_vertices.emplace_back(Vertex{glm::vec3(-1.0f, 1.0f, 1.0f), glm::vec4(1.0f, 1.0f, 0.5f, 0.0f)});
        m_vertices.emplace_back(Vertex{glm::vec3(1.0f, 1.0f, 1.0f), glm::vec4(1.0f, 1.0f, 0.5f, 0.0f)});
        m_vertices.emplace_back(Vertex{glm::vec3(-1.0f, -1.0f, 1.0f), glm::vec4(1.0f, 1.0f, 0.5f, 0.0f)});
        m_vertices.emplace_back(Vertex{glm::vec3(1.0f, -1.0f, 1.0f), glm::vec4(1.0f, 1.0f, 0.5f, 0.0f)});

        m_gridData.resize(GRID_SIZE);
        glm::mat4 model = glm::mat4(1);
        size_t indexTemp = 0;
        for (size_t i = 0; i < GRID_ROW; i++)
        {
            for (size_t j = 0; j < GRID_COLUMN; j++)
            {
                m_statsObjectsRendered++;
                glm::mat4 transform = glm::translate(model, glm::vec3(4.0f * i, 0.0f, -4.0f * j));
                m_gridData[indexTemp].position = glm::vec3(4.0f * i, 0.0f, -4.0f * j);
                transform = glm::scale(transform, glm::vec3(1.0, 0.5, 1.0));
                m_gridData[indexTemp].transform = transform;
                m_gridData[indexTemp].index = indexTemp;
                indexTemp++;
            }
        }

        m_obstacles.resize(m_obstaclesIndexes.size());
        size_t currentObstacleIndex = 0;
        for (auto &index : m_obstaclesIndexes)
        {
            m_statsObjectsRendered++;
            m_obstacles[currentObstacleIndex].position = m_gridData[index].position;
            m_obstacles[currentObstacleIndex].transform = glm::translate(glm::mat4(1), m_obstacles[currentObstacleIndex].position + glm::vec3(0.0f, 1.5f, 0.0f));
            currentObstacleIndex++;
        }


        std::vector<float> randomColors;
        for (int c = 0; c <= 255; c += (GRID_ROW/NUMBER_OF_AGENTS))
        {
            randomColors.push_back(c);
        }
        std::random_shuffle(randomColors.begin(), randomColors.end());

        m_agents.resize(NUMBER_OF_AGENTS);
        for (int i = 0; i < m_agents.size(); i++)
        {
            m_statsObjectsRendered++;
            m_agents[i].color = glm::vec4(((float)randomColors[m_agents.size() - 1 - i])/ 255.0f, ((float)randomColors[i + 1]) / 255.0f, ((float)randomColors[i]) / 255.0f, 1.0f);
            m_agents[i].position = m_gridData[m_grid[i].start].position;
            m_agents[i].transform = glm::translate(glm::mat4(1), m_agents[i].position + glm::vec3(0.0f, 1.5f, 0.0f));
        }
    }
    void Application::CreateDescriptorPool()
    {
        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = (2 * m_gridData.size()) + (2 * m_agents.size()) + (2 * m_obstacles.size()) + 2;

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.maxSets = (2 * m_gridData.size()) + (2 * m_agents.size()) +  (2*m_obstacles.size()) + 2;

        VK_CHECK_RESULT(vkCreateDescriptorPool(m_device->LogicalDeviceHandle(), &poolInfo, nullptr, &m_descriptorPool));
    }
    void Application::CreateDescriptorSets()
    {

        std::vector<VkDescriptorSetLayout> layouts{m_defaultPipelineSpec.descriptorSetLayouts[0], m_defaultPipelineSpec.descriptorSetLayouts[1]};

        for (size_t i = 0; i < 2; i++)
        {
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = m_descriptorPool;
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts = &layouts[0];
            vkAllocateDescriptorSets(m_device->LogicalDeviceHandle(), &allocInfo, &m_cameraData.descriptors[i]);
        }

        for (size_t i = 0; i < 2; i++)
        {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = m_cameraData.cameraUBOs[i]->BufferHandle();
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(CameraUBO::Values);

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = m_cameraData.descriptors[i];
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &bufferInfo;
            vkUpdateDescriptorSets(m_device->LogicalDeviceHandle(), 1, &descriptorWrite, 0, nullptr);
        }

        for (size_t i = 0; i < m_swapchain->MAX_FRAMES_IN_FLIGHT; i++)
        {
            for (size_t j = 0; j < m_gridData.size(); j++)
            {

                VkDescriptorSetAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.descriptorPool = m_descriptorPool;
                allocInfo.descriptorSetCount = 1;
                allocInfo.pSetLayouts = &layouts[1];
                vkAllocateDescriptorSets(m_device->LogicalDeviceHandle(), &allocInfo, &m_gridData[j].descriptors[i]);
            }
        }

        for (size_t i = 0; i < m_swapchain->MAX_FRAMES_IN_FLIGHT; i++)
        {
            for (size_t j = 0; j < m_gridData.size(); j++)
            {

                VkDescriptorBufferInfo bufferInfo{};
                bufferInfo.buffer = m_gridData[j].modelUBOs[i]->BufferHandle();
                bufferInfo.offset = 0;
                bufferInfo.range = sizeof(glm::mat4);

                VkWriteDescriptorSet descriptorWrite{};
                descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrite.dstSet = m_gridData[j].descriptors[i];
                descriptorWrite.dstBinding = 0;
                descriptorWrite.dstArrayElement = 0;
                descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                descriptorWrite.descriptorCount = 1;
                descriptorWrite.pBufferInfo = &bufferInfo;
                vkUpdateDescriptorSets(m_device->LogicalDeviceHandle(), 1, &descriptorWrite, 0, nullptr);
            }
        }


        for (size_t i = 0; i < m_swapchain->MAX_FRAMES_IN_FLIGHT; i++)
        {
            for (size_t j = 0; j < m_obstacles.size(); j++)
            {

                VkDescriptorSetAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.descriptorPool = m_descriptorPool;
                allocInfo.descriptorSetCount = 1;
                allocInfo.pSetLayouts = &layouts[1];
                vkAllocateDescriptorSets(m_device->LogicalDeviceHandle(), &allocInfo, &m_obstacles[j].descriptors[i]);
            }
        }

        for (size_t i = 0; i < m_swapchain->MAX_FRAMES_IN_FLIGHT; i++)
        {
            for (size_t j = 0; j < m_obstacles.size(); j++)
            {

                VkDescriptorBufferInfo bufferInfo{};
                bufferInfo.buffer = m_obstacles[j].modelUBOs[i]->BufferHandle();
                bufferInfo.offset = 0;
                bufferInfo.range = sizeof(glm::mat4);

                VkWriteDescriptorSet descriptorWrite{};
                descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrite.dstSet = m_obstacles[j].descriptors[i];
                descriptorWrite.dstBinding = 0;
                descriptorWrite.dstArrayElement = 0;
                descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                descriptorWrite.descriptorCount = 1;
                descriptorWrite.pBufferInfo = &bufferInfo;
                vkUpdateDescriptorSets(m_device->LogicalDeviceHandle(), 1, &descriptorWrite, 0, nullptr);
            }
        }

        for (size_t i = 0; i < m_swapchain->MAX_FRAMES_IN_FLIGHT; i++)
        {
            for (size_t j = 0; j < m_agents.size(); j++)
            {

                VkDescriptorSetAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.descriptorPool = m_descriptorPool;
                allocInfo.descriptorSetCount = 1;
                allocInfo.pSetLayouts = &layouts[1];
                vkAllocateDescriptorSets(m_device->LogicalDeviceHandle(), &allocInfo, &m_agents[j].descriptors[i]);
            }
        }

        for (size_t i = 0; i < m_swapchain->MAX_FRAMES_IN_FLIGHT; i++)
        {
            for (size_t j = 0; j < m_agents.size(); j++)
            {

                VkDescriptorBufferInfo bufferInfo{};
                bufferInfo.buffer = m_agents[j].modelUBOs[i]->BufferHandle();
                bufferInfo.offset = 0;
                bufferInfo.range = sizeof(glm::mat4);

                VkWriteDescriptorSet descriptorWrite{};
                descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrite.dstSet = m_agents[j].descriptors[i];
                descriptorWrite.dstBinding = 0;
                descriptorWrite.dstArrayElement = 0;
                descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                descriptorWrite.descriptorCount = 1;
                descriptorWrite.pBufferInfo = &bufferInfo;
                vkUpdateDescriptorSets(m_device->LogicalDeviceHandle(), 1, &descriptorWrite, 0, nullptr);
            }
        }
    }

    void Application::CreateComputePipelineLayout()
    {

        {
            VkDescriptorSetLayout descriptorSetLayout;
            VkDescriptorSetLayoutBinding layoutBindings;
            layoutBindings.binding = 0;
            layoutBindings.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            layoutBindings.descriptorCount = 1;
            layoutBindings.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = 1;
            layoutInfo.pBindings = &layoutBindings;

            VK_CHECK_RESULT(vkCreateDescriptorSetLayout(m_device->LogicalDeviceHandle(), &layoutInfo, nullptr, &descriptorSetLayout));

            m_computeSetLayouts.emplace_back(descriptorSetLayout);
        }

        {
            VkDescriptorSetLayout descriptorSetLayout;
            VkDescriptorSetLayoutBinding layoutBindings;
            layoutBindings.binding = 0;
            layoutBindings.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            layoutBindings.descriptorCount = 1;
            layoutBindings.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = 1;
            layoutInfo.pBindings = &layoutBindings;

            VK_CHECK_RESULT(vkCreateDescriptorSetLayout(m_device->LogicalDeviceHandle(), &layoutInfo, nullptr, &descriptorSetLayout));

            m_computeSetLayouts.emplace_back(descriptorSetLayout);
        }

        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
        pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.setLayoutCount = m_computeSetLayouts.size();
        pipelineLayoutCreateInfo.pSetLayouts = m_computeSetLayouts.data();

        VK_CHECK_RESULT(vkCreatePipelineLayout(m_device->LogicalDeviceHandle(), &pipelineLayoutCreateInfo, nullptr, &m_pipelineLayoutCompute));
    }

    void Application::CreateComputePipeline(const std::string &shaderPath)
    {
        VkShaderModuleCreateInfo shaderModuleCreateInfo{};
        shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

        auto shaderCode = VulkanPipeline::ReadShaderFile(shaderPath);
        shaderModuleCreateInfo.pCode = reinterpret_cast<uint32_t *>(shaderCode.data());
        shaderModuleCreateInfo.codeSize = shaderCode.size();

        VkShaderModule shaderModule = VK_NULL_HANDLE;
        VK_CHECK_RESULT(vkCreateShaderModule(m_device->LogicalDeviceHandle(), &shaderModuleCreateInfo, nullptr, &shaderModule));

        VkComputePipelineCreateInfo pipelineCreateInfo{};
        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        pipelineCreateInfo.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        pipelineCreateInfo.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        pipelineCreateInfo.stage.module = shaderModule;

        pipelineCreateInfo.stage.pName = "main";
        pipelineCreateInfo.layout = m_pipelineLayoutCompute;

        VK_CHECK_RESULT(vkCreateComputePipelines(m_device->LogicalDeviceHandle(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &m_computePipeline));

        vkDestroyShaderModule(m_device->LogicalDeviceHandle(), shaderModule, nullptr);
    }

    void Application::CreateComputeStorageBuffers()
    {
        VkDeviceSize bufferSize = sizeof(GridData) * NUMBER_OF_AGENTS;
        VkDeviceSize bufferSize2 = sizeof(Path) * NUMBER_OF_AGENTS;


        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        bufferInfo.size = bufferSize;

        VkBufferCreateInfo bufferInfo2 = {};
        bufferInfo2.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo2.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        bufferInfo2.size = bufferSize2;

        m_ssbObjects[0].buffer.allocationHandle = m_allocator->AllocateBuffer(&bufferInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, &m_ssbObjects[0].buffer.bufferHandle);
        m_ssbObjects[0].buffer.data = m_allocator->MapMemory(m_ssbObjects[0].buffer.allocationHandle);

        m_ssbObjects[1].buffer.allocationHandle = m_allocator->AllocateBuffer(&bufferInfo2, VMA_MEMORY_USAGE_CPU_TO_GPU, &m_ssbObjects[1].buffer.bufferHandle);
        m_ssbObjects[1].buffer.data = m_allocator->MapMemory(m_ssbObjects[1].buffer.allocationHandle);
    }

    void Application::CreateComputeDescriptorPool()
    {
        VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
        descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolCreateInfo.maxSets = 2;

        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        poolSize.descriptorCount = static_cast<uint32_t>(1);

        descriptorPoolCreateInfo.poolSizeCount = 1;
        descriptorPoolCreateInfo.pPoolSizes = &poolSize;
        VK_CHECK_RESULT(vkCreateDescriptorPool(m_device->LogicalDeviceHandle(), &descriptorPoolCreateInfo, nullptr, &m_descriptorPoolCompute));

        VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
        descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptorSetAllocateInfo.descriptorPool = m_descriptorPoolCompute;
        descriptorSetAllocateInfo.descriptorSetCount = 1;
        descriptorSetAllocateInfo.pSetLayouts = m_computeSetLayouts.data();

        for (int i = 0; i < 2; i++)
        {
            VK_CHECK_RESULT(vkAllocateDescriptorSets(m_device->LogicalDeviceHandle(), &descriptorSetAllocateInfo, &m_ssbObjects[i].descriptor));

            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = m_ssbObjects[i].buffer.bufferHandle;
            bufferInfo.offset = 0;
            bufferInfo.range = VK_WHOLE_SIZE;

            VkWriteDescriptorSet writeDescriptorSet{};
            writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptorSet.dstSet = m_ssbObjects[i].descriptor;
            writeDescriptorSet.dstBinding = 0;
            writeDescriptorSet.dstArrayElement = 0;
            writeDescriptorSet.descriptorCount = 1;
            writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            writeDescriptorSet.pBufferInfo = &bufferInfo;

            vkUpdateDescriptorSets(m_device->LogicalDeviceHandle(), 1, &writeDescriptorSet, 0, nullptr);
        }
    }
    void Application::CreateComputeCommandPool()
    {

        VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
        commandBufferAllocateInfo.sType =
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateInfo.commandPool = m_device->CommandPoolHandle();
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocateInfo.commandBufferCount = 1;

        VK_CHECK_RESULT(vkAllocateCommandBuffers(m_device->LogicalDeviceHandle(), &commandBufferAllocateInfo, &m_computeCommandBuffer));
    }

    void Application::BuildComputeCommands()
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(m_computeCommandBuffer, &beginInfo);
        vkCmdBindPipeline(m_computeCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_computePipeline);
        vkCmdBindDescriptorSets(m_computeCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipelineLayoutCompute, 0, 1, &m_ssbObjects[0].descriptor, 0, nullptr);
        vkCmdBindDescriptorSets(m_computeCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipelineLayoutCompute, 1, 1, &m_ssbObjects[1].descriptor, 0, nullptr);
        vkCmdDispatch(m_computeCommandBuffer, NUMBER_OF_AGENTS, 1, 1);
        if (vkEndCommandBuffer(m_computeCommandBuffer) != VK_SUCCESS)
        {
            APP_ERROR("failed to end command buffer");
        }

        m_gridGPU = new GridData[NUMBER_OF_AGENTS];

        for (int i = 0; i < NUMBER_OF_AGENTS; i++)
        {
            m_gridGPU[i] = m_grid[i];
        }
        memcpy(m_ssbObjects[0].buffer.data, m_gridGPU, sizeof(GridData) * NUMBER_OF_AGENTS);

        m_pathGPU = new Path[NUMBER_OF_AGENTS];
        for (int i = 0; i < NUMBER_OF_AGENTS; i++)
        {
            for (int j = 0; j < GRID_SIZE; j++)
            {
                m_pathGPU[i].pathList[j] = -1;
            }
        }
        memcpy(m_ssbObjects[1].buffer.data, m_pathGPU, sizeof(Path) * NUMBER_OF_AGENTS);
    }

    void Application::CalculateCompute()
    {
        {
            VkSubmitInfo submitInfo2 = {};
            submitInfo2.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo2.commandBufferCount = 1;
            submitInfo2.pCommandBuffers = &m_computeCommandBuffer;
            vkQueueSubmit(m_device->GraphicsQueueHandle(), 1, &submitInfo2, VK_NULL_HANDLE);
            Timer time(true, "GPU Pathfinding");
            vkQueueWaitIdle(m_device->GraphicsQueueHandle());
        }

        Path *path = (Path *)m_ssbObjects[1].buffer.data;

        for (int i = 0; i < NUMBER_OF_AGENTS; i++)
        {
            std::vector<int> temp;
            for (int j = 0; j < GRID_SIZE; j++)
            {
                if (path[i].pathList[j] != -1)
                {
                    temp.push_back(path[i].pathList[j]);
                }
            }
            std::reverse(temp.begin(), temp.end());
            m_agents[i].path = temp;
        }
    }

    void Application::InitGrids()
    {
        GridData grid;
        srand(time(nullptr));
        for (int i = 0; i < GRID_ROW; i++)
        {
            for (int j = 0; j < GRID_COLUMN; j++)
            {
                int index = AStar::FindIndex(i, j);
                grid.nodes[index].previousNode = -1;
                grid.nodes[index].i = i;
                grid.nodes[index].j = j;
                grid.nodes[index].ID = index;

                grid.nodes[index].Fcost = 0.0f;
                grid.nodes[index].Hcost = 0.0f;
                grid.nodes[index].Gcost = 0.0f;
                if ((i > 0) && (i < GRID_ROW - 1) && (j >= 0) && (j < GRID_COLUMN))
                {
                    if ((rand() % 3) == 0)
                    {
                        grid.nodes[index].passable = 0;
                        m_obstaclesIndexes.push_back(index);
                    }
                    else
                    {
                        grid.nodes[index].passable = 1;
                    }
                }
                else
                {
                    grid.nodes[index].passable = 1;
                }

                for (int i = 0; i < 8; i++)
                {
                    grid.nodes[index].neighbours[i] = -1;
                }

                if (i < GRID_ROW - 1)
                {
                    grid.nodes[index].neighbours[0] = AStar::FindIndex(i + 1, j);
                }
                if (i > 0)
                {
                    grid.nodes[index].neighbours[1] = AStar::FindIndex(i - 1, j);
                }
                if (j < GRID_COLUMN - 1)
                {
                    grid.nodes[index].neighbours[2] = AStar::FindIndex(i, j + 1);
                }
                if (j > 0)
                {
                    grid.nodes[index].neighbours[3] = AStar::FindIndex(i, j - 1);
                }

                if ((i < GRID_ROW - 1) && (j < GRID_COLUMN - 1))
                {
                    grid.nodes[index].neighbours[4] = AStar::FindIndex(i + 1, j + 1);
                }

                if ((i < GRID_ROW - 1) && (j > 0))
                {
                    grid.nodes[index].neighbours[5] = AStar::FindIndex(i + 1, j - 1);
                }

                if ((i > 0) && (j > 0))
                {
                    grid.nodes[index].neighbours[6] = AStar::FindIndex(i - 1, j - 1);
                }

                if ((i > 0) && (j < GRID_COLUMN - 1))
                {
                    grid.nodes[index].neighbours[7] = AStar::FindIndex(i - 1, j + 1);
                }
            }
        }

        std::vector<int> randomNumbers;
        for (int i = 0; i < GRID_COLUMN; i++)
        {
            randomNumbers.emplace_back(i);
        }
        std::random_shuffle(randomNumbers.begin(), randomNumbers.end());

        for (int k = 0; k < NUMBER_OF_AGENTS; k++)
        {
            m_grid.push_back(grid);
            int firstNumber = randomNumbers[k];
            int secondNumber = randomNumbers[randomNumbers.size() - 1 - k];
            m_grid[k].start = AStar::FindIndex(0, firstNumber);
            m_grid[k].end = AStar::FindIndex(GRID_ROW - 1, secondNumber);
        }
    }

    void Application::ResolvePath()
    {
        Timer cpuPathfindingTimer(true, "CPU Pathfinding");
        for (int k = 0; k < NUMBER_OF_AGENTS; k++)
        {
            auto result = AStar::FindPath(m_grid[k]);
            m_agents[k].path = result;
        }
        _time = cpuPathfindingTimer.GetElapsedMiliseconds();
    }
}
