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
        // Timer timer(true);
        Initialize();
        InitObjects();


        CreateVertexBuffer();
        CreateIndexBuffer();
        CreateUniformBuffer();
        CreateDescriptorPool();
        CreateDescriptorSets();


        // Init Vulkan
        while (m_window->IsOpen())
        {

            // Delta Time
            m_deltaTime.Update(static_cast<float>(glfwGetTime()));
            // APP_TRACE("Delta Time: {}", m_deltaTime.AsMiliSeconds());
            // Delta Time

            // FRAME START
            // dooo update
            // doo render

            // auto [x,y] = Input::MousePosition();
            // APP_INFO("MOUSE_POSITION  X:{} Y:{}",x,y);
            // FRAME END
            for(int i=0;i<20;i++){
            UpdateUniformBuffer(i);
            }
            Draw();
            m_window->ProcessEvents(); // process events/inputs

            m_fpsCounter.Update();

            // if(m_fpsCounter.GetFPS()>0)
            //     APP_TRACE("FPS {}", m_fpsCounter.GetFPS());
        }
        Shutdown();
    }

    void Application::Initialize()
    {

        std::filesystem::path cwd = std::filesystem::current_path();
        std::cout << cwd << '\n';

        Timer timer(true, GET_NAME(Initialize()));

        Logger::Init();

        m_window = std::make_shared<Window>(1600, 900, std::move("Pathfinding"));
        m_context = std::make_unique<VulkanContext>();

        m_device = std::make_unique<VulkanDevice>();
        m_allocator = std::make_unique<VulkanAllocator>(*m_device);

        m_swapchain = std::make_unique<VulkanSwapChain>(*m_device);

        m_defaultPipelineSpec = VulkanPipeline::DefaultPipelineSpecification(m_swapchain->RenderPassHandle());
        m_defaultPipelineSpec.vertexInputDescription = VertexInputDescription::GetVertexDescription();
        m_defaultPipelineSpec.pushConstantData = std::make_unique<PipelinePushConstantData>(PipelinePushConstantData{glm::mat4(1.0f)});
        m_defaultPipelineSpec.AddDescriptorSetLayout(*m_device);
        m_defaultPipline = std::make_unique<VulkanPipeline>(*m_device, "../shaders/test.vert.spv", "../shaders/test.frag.spv", m_defaultPipelineSpec);

        CreateCommandBuffers();
    }
    void Application::Shutdown()
    {
        m_allocator->DestroyBuffer(m_vertexBuffer.bufferHandle, m_vertexBuffer.allocationHandle);
        vkDeviceWaitIdle(m_device->LogicalDeviceHandle());
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
void Application::Draw()
    {
        uint32_t imageIndex;
        VkResult acquireResult = m_swapchain->AcquireNextImage(&imageIndex);
        if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR)
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

        if (Input::KeyPressed(GLFW_KEY_W))
        {
            clearColor = {{{0.025f, 0.5f, 0.025f, 1.0f}}};
        }
        else
        {
            clearColor = {{{0.025f, 0.025f, 0.025f, 1.0f}}};
        }

        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        // Begin Render Pass
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

        // calculate final mesh matrix
        //view = glm::translate(glm::mat4(1.f), camPos);
        //model = glm::rotate(model,glm::radians(1.0f), glm::vec3(0.0f,0.0f,1.0f));
        //model = glm::translate(model, glm::vec3(0.0f, 0.0f, -m_deltaTime.AsSeconds()));
        //glm::mat4 mesh_matrix = projection * view * model;

        


        // upload the matrix to the GPU via push constants

        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_vertexBuffer.bufferHandle, &offset);
        vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer.bufferHandle,0, VK_INDEX_TYPE_UINT32);
        //vkCmdDraw(commandBuffer, m_vertices.size(), 1, 0, 0);
        for(int i=0;i<20;i++){
            PipelinePushConstantData constant;
            constant.projection = glm::mat4(1);

            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_defaultPipline->PipelineLayoutHandle(), 0, 1, &m_descriptors[i], 0, nullptr);
            vkCmdPushConstants(commandBuffer, m_defaultPipline->PipelineLayoutHandle(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PipelinePushConstantData), &constant);
            vkCmdDrawIndexed(commandBuffer, m_indices.size(), 1, 0, 0, 0);
        }

        //for(int i=0;i<1;i++){
        //    //if(Input::KeyPressed(GLFW_KEY_W)){
        //    constant.projection = transforms[i];
        //    //}
        //    vkCmdPushConstants(commandBuffer, m_defaultPipline->PipelineLayoutHandle(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PipelinePushConstantData), &constant);
        //    // calculate final mesh matrix
        //    //view = glm::translate(glm::mat4(1.f), camPos);
        //    //model = glm::rotate(model,glm::radians(1.0f), glm::vec3(0.0f,0.0f,1.0f));
        //    //model = glm::translate(model, glm::vec3(0.0f, 0.0f, -m_deltaTime.AsSeconds()));
        //    //glm::mat4 mesh_matrix = projection * view * model;
        //    vkCmdDrawIndexed(commandBuffer,m_indices.size(),1,0,0,0);
        //}

       

        vkCmdEndRenderPass(commandBuffer);
        // End Render Pass
        vkEndCommandBuffer(commandBuffer);

        VkResult submitResult = m_swapchain->SubmitCommandBuffers(&commandBuffer, &imageIndex);
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

    void Application::InitObjects()
    {
        m_vertices.emplace_back(Vertex{glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec4(1.0f, 1.0f, 0.5f, 0.0f)});
        m_vertices.emplace_back(Vertex{glm::vec3(0.5f, -0.5f, 0.0f), glm::vec4(1.0f, 1.0f, 0.5f, 0.0f)});
        m_vertices.emplace_back(Vertex{glm::vec3(0.5f, 0.5f, 0.0f), glm::vec4(1.0f, 1.0f, 0.5f, 0.0f)});
        m_vertices.emplace_back(Vertex{glm::vec3(-0.5f, 0.5f, 0.0f), glm::vec4(1.0f, 1.0f, 0.5f, 0.0f)});

        transforms.resize(10);
        for(int i=0;i<10;i++){
            transforms[i] =glm::mat4(1);
        }   
    }

    void Application::CreateVertexBuffer()
    {
        // m_vertexBuffer.allocationHandle

        VkBufferCreateInfo bufferCreateInfo1 = VulkanInitializers::BufferCreateInfo(
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_SHARING_MODE_EXCLUSIVE, m_vertices.size() * sizeof(Vertex));
        m_vertexBuffer.allocationHandle = m_allocator->AllocateBuffer(&bufferCreateInfo1, VMA_MEMORY_USAGE_CPU_TO_GPU, &m_vertexBuffer.bufferHandle);

        VkBufferCreateInfo bufferCreateInfo{};

        bufferCreateInfo = VulkanInitializers::BufferCreateInfo(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                                VK_SHARING_MODE_EXCLUSIVE,
                                                                m_vertices.size() * sizeof(Vertex));
        VulkanBuffer stagingBuffer;

        VmaAllocation stagingBufferAllocation =
            m_allocator->AllocateBuffer(
                &bufferCreateInfo,
                VMA_MEMORY_USAGE_CPU_TO_GPU,
                &stagingBuffer.bufferHandle);

        stagingBuffer.data = m_allocator->MapMemory(stagingBufferAllocation);
        memcpy(stagingBuffer.data, m_vertices.data(), m_vertices.size() * sizeof(Vertex));
        m_allocator->UnmapMemory(stagingBufferAllocation);

        VkCommandPool cmdPool;

        VkCommandPoolCreateInfo cmdPoolCreateInfo{};
        cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cmdPoolCreateInfo.queueFamilyIndex = m_device->GraphicsQueueFamilyIndex();
        cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

        VK_CHECK_RESULT(vkCreateCommandPool(m_device->LogicalDeviceHandle(), &cmdPoolCreateInfo, VK_NULL_HANDLE, &cmdPool));

        VkCommandBuffer cmdBuffer;

        VkCommandBufferAllocateInfo cmdBufferAllocateInfo{};
        cmdBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmdBufferAllocateInfo.commandPool = cmdPool;
        cmdBufferAllocateInfo.commandBufferCount = 1;
        cmdBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        vkAllocateCommandBuffers(m_device->LogicalDeviceHandle(), &cmdBufferAllocateInfo, &cmdBuffer);

        VkCommandBufferBeginInfo commandBufferBeginInfo = {};
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(cmdBuffer, &commandBufferBeginInfo);
        VkBufferCopy bufferCopy = {};
        bufferCopy.size = m_vertices.size() * sizeof(Vertex);
        vkCmdCopyBuffer(cmdBuffer, stagingBuffer.bufferHandle, m_vertexBuffer.bufferHandle, 1, &bufferCopy);
        vkEndCommandBuffer(cmdBuffer);


        VkFence fence;
        VkFenceCreateInfo fenceCreateInfo = {};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        VK_CHECK_RESULT(vkCreateFence(m_device->LogicalDeviceHandle(), &fenceCreateInfo, nullptr, &fence));

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmdBuffer;

        VK_CHECK_RESULT(vkQueueSubmit(m_device->GraphicsQueueHandle(), 1, &submitInfo, fence));
        vkWaitForFences(m_device->LogicalDeviceHandle(), 1, &fence, VK_TRUE, UINT64_MAX);

        vkDestroyCommandPool(m_device->LogicalDeviceHandle(), cmdPool, nullptr);
        m_allocator->DestroyBuffer(stagingBuffer.bufferHandle, stagingBufferAllocation);
        vkDestroyFence(m_device->LogicalDeviceHandle(), fence, nullptr);
    }
    void Application::CreateIndexBuffer()
    {

        VkBufferCreateInfo bufferCreateInfo1 = VulkanInitializers::BufferCreateInfo(
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_SHARING_MODE_EXCLUSIVE,
            sizeof(m_indices[0]) * m_indices.size());
        m_vertexBuffer.allocationHandle = m_allocator->AllocateBuffer(&bufferCreateInfo1, VMA_MEMORY_USAGE_CPU_TO_GPU, &m_indexBuffer.bufferHandle);

        VkBufferCreateInfo bufferCreateInfo{};

        bufferCreateInfo = VulkanInitializers::BufferCreateInfo(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                                VK_SHARING_MODE_EXCLUSIVE,
                                                                sizeof(m_indices[0]) * m_indices.size());
        VulkanBuffer stagingBuffer;

        VmaAllocation stagingBufferAllocation =
            m_allocator->AllocateBuffer(
                &bufferCreateInfo,
                VMA_MEMORY_USAGE_CPU_TO_GPU,
                &stagingBuffer.bufferHandle);

        stagingBuffer.data = m_allocator->MapMemory(stagingBufferAllocation);
        memcpy(stagingBuffer.data, m_indices.data(), sizeof(m_indices[0]) * m_indices.size());
        m_allocator->UnmapMemory(stagingBufferAllocation);

        VkCommandPool cmdPool;

        VkCommandPoolCreateInfo cmdPoolCreateInfo{};
        cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cmdPoolCreateInfo.queueFamilyIndex = m_device->GraphicsQueueFamilyIndex();
        cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

        VK_CHECK_RESULT(vkCreateCommandPool(m_device->LogicalDeviceHandle(), &cmdPoolCreateInfo, VK_NULL_HANDLE, &cmdPool));

        VkCommandBuffer cmdBuffer;

        VkCommandBufferAllocateInfo cmdBufferAllocateInfo{};
        cmdBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmdBufferAllocateInfo.commandPool = cmdPool;
        cmdBufferAllocateInfo.commandBufferCount = 1;
        cmdBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        vkAllocateCommandBuffers(m_device->LogicalDeviceHandle(), &cmdBufferAllocateInfo, &cmdBuffer);

        VkCommandBufferBeginInfo commandBufferBeginInfo = {};
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(cmdBuffer, &commandBufferBeginInfo);
        VkBufferCopy bufferCopy = {};
        bufferCopy.size = sizeof(m_indices[0]) * m_indices.size();
        vkCmdCopyBuffer(cmdBuffer, stagingBuffer.bufferHandle, m_indexBuffer.bufferHandle, 1, &bufferCopy);
        vkEndCommandBuffer(cmdBuffer);

        VkFence fence;
        VkFenceCreateInfo fenceCreateInfo = {};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        VK_CHECK_RESULT(vkCreateFence(m_device->LogicalDeviceHandle(), &fenceCreateInfo, nullptr, &fence));

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmdBuffer;

        VK_CHECK_RESULT(vkQueueSubmit(m_device->GraphicsQueueHandle(), 1, &submitInfo, fence));
        vkWaitForFences(m_device->LogicalDeviceHandle(), 1, &fence, VK_TRUE, UINT64_MAX);

        vkDestroyCommandPool(m_device->LogicalDeviceHandle(), cmdPool, nullptr);
        m_allocator->DestroyBuffer(stagingBuffer.bufferHandle, stagingBufferAllocation);
        vkDestroyFence(m_device->LogicalDeviceHandle(), fence, nullptr);
    }

    void Application::CreateUniformBuffer()
    {
        VkDeviceSize bufferSize = sizeof(UniformBuffer::Values);

        m_uniformBuffers.resize(20);
        
        for (size_t i = 0; i < 20; i++){
        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.pNext = nullptr;
        allocInfo.allocationSize = 0;
        allocInfo.memoryTypeIndex = 0;

        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        bufferInfo.size = bufferSize;
        m_uniformBuffers[i].allocationHandle = m_allocator->AllocateBuffer(&bufferInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, &m_uniformBuffers[i].bufferHandle);
        m_uniformBuffers[i].data = m_allocator->MapMemory(m_uniformBuffers[i].allocationHandle);
    }
    }
    void Application::UpdateUniformBuffer(uint32_t currentFrame)
    {


        if (Input::KeyPressed(GLFW_KEY_D))
        {
            view = glm::translate(view, glm::vec3(-0.0001f, 0.0, 0.0));
          
        }
        if(Input::KeyPressed(GLFW_KEY_S)){
            view = glm::translate(view, glm::vec3(0.0f, -0.00001f, 0.0));
        }

        UniformBuffer::Values uboValues{};
        uboValues.model = model;
        uboValues.proj = projection;
        uboValues.view = view;

        transforms[currentFrame] =glm::translate(transforms[currentFrame], glm::vec3((m_deltaTime.AsSeconds()*0.1f)* (currentFrame+1), m_deltaTime.AsSeconds()*(0.01f) * (currentFrame+1), -((m_deltaTime.AsSeconds()*0.01f)* (currentFrame+1))));
        uboValues.model = transforms[currentFrame];

        memcpy(m_uniformBuffers[currentFrame].data, &uboValues, sizeof(UniformBuffer::Values));

        //for(int i=0;i<10;i++){
        //  = glm::translate(transforms[i], glm::vec3(0.0001f, 0.0f, 0.0f));
        //}


    }
    void Application::CreateDescriptorPool()
    {
        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = static_cast<uint32_t>(20);

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.maxSets = 1000;

        VK_CHECK_RESULT(vkCreateDescriptorPool(m_device->LogicalDeviceHandle(), &poolInfo, nullptr, &m_descriptorPool));
    }
    void Application::CreateDescriptorSets()
    {
        std::vector<VkDescriptorSetLayout> layouts(20, m_defaultPipelineSpec.descriptorSetLayouts[0]);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(20);
        allocInfo.pSetLayouts = layouts.data();

        m_descriptors.resize(20);
        vkAllocateDescriptorSets(m_device->LogicalDeviceHandle(), &allocInfo, m_descriptors.data());

        for (size_t i = 0; i < 20; i++)
        {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = m_uniformBuffers[i].bufferHandle;
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBuffer::Values);

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = m_descriptors[i];
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &bufferInfo;

            vkUpdateDescriptorSets(m_device->LogicalDeviceHandle(), 1, &descriptorWrite, 0, nullptr);
        }
    }
}
