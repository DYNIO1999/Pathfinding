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

        CreateCommandBuffers();
        CreateVertexBuffer();
        CreateIndexBuffer();
        CreateUniformBuffer();
        CreateDescriptorPool();
        CreateDescriptorSets();

        CreateComputePipelineLayout();
        CreateComputePipeline("../shaders/test.comp.spv");
        CreateComputeStorageBuffers();
        CreateComputeDescriptorPool();

        CreateComputeCommandPool();
        BuildComputeCommands();

        CalculateCompute();

        TestingAbstraction();
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

         
            m_camera->Update(m_deltaTime.AsSeconds());
            UpdateUniformBuffer(m_swapchain->CurrentFrame());

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

        m_swapchain = std::make_unique<VulkanSwapChain>(*m_device, *m_allocator);

        m_defaultPipelineSpec = VulkanPipeline::DefaultPipelineSpecification(m_swapchain->RenderPassHandle());
        
        VulkanVertexInputDescription vertexDescription = VulkanVertexInputDescription::Create().
                                                        AddBinding(0,sizeof(Vertex),VK_VERTEX_INPUT_RATE_VERTEX).
                                                        AddAttribute(0,0,VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex,position)).
                                                        AddAttribute(0,1,VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex,color));


        m_defaultPipelineSpec.vertexInputDescription = vertexDescription;
        m_defaultPipelineSpec.pushConstantData = std::make_unique<PipelinePushConstantData>(PipelinePushConstantData{glm::vec4(1.0f, 1.0f, 0.5f, 0.0f)});
        m_defaultPipelineSpec.AddDescriptorSetLayout(*m_device);
        m_defaultPipline = std::make_unique<VulkanPipeline>(*m_device, "../shaders/test.vert.spv", "../shaders/test.frag.spv", m_defaultPipelineSpec);


        auto [width,height ] = Application::GetWindow()->WindowSize();
        m_camera = std::make_unique<Camera>(static_cast<float>(width), static_cast<float>(height));

    }
    void Application::Shutdown()
    {     
        vkDeviceWaitIdle(m_device->LogicalDeviceHandle());


        for(int i=0;i<2;i++){
            m_allocator->UnmapMemory(m_ssbObjects[i].buffer.allocationHandle);
            m_allocator->DestroyBuffer(m_ssbObjects[i].buffer.bufferHandle,m_ssbObjects[i].buffer.allocationHandle);
        }
        

        for (size_t i = 0; i < 2; i++)
        {
            for (size_t j = 0; j < m_objectsData.size(); j++)
            {
                m_allocator->UnmapMemory(m_objectsData[j].modelUBOs[i].buffer.allocationHandle);
                m_allocator->DestroyBuffer(m_objectsData[j].modelUBOs[i].buffer.bufferHandle, m_objectsData[j].modelUBOs[i].buffer.allocationHandle);
            }
        }

        for(auto it: m_defaultPipelineSpec.descriptorSetLayouts){
            vkDestroyDescriptorSetLayout(m_device->LogicalDeviceHandle(), it, nullptr);
        }

        vkDestroyDescriptorPool(m_device->LogicalDeviceHandle(),m_descriptorPool,nullptr);
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

        // Render Pass
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

        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_vertexBuffer->BufferHandle(), &offset);
        vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer->BufferHandle(),0, VK_INDEX_TYPE_UINT32);
        //vkCmdDraw(commandBuffer, m_vertices.size(), 1, 0, 0);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_defaultPipline->PipelineLayoutHandle(), 0, 1, &m_cameraData.descriptors[m_swapchain->CurrentFrame()], 0, nullptr);

        for (size_t i = 0; i < m_objectsData.size(); i++)
        {
            
            PipelinePushConstantData constant;
            if(i%2==0){
            constant.color = glm::vec4(0.5,1.0f,0.7f,1.0f);
            }else{
                constant.color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
            }
            
            vkCmdPushConstants(commandBuffer, m_defaultPipline->PipelineLayoutHandle(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PipelinePushConstantData), &constant);
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_defaultPipline->PipelineLayoutHandle(), 1, 1,&m_objectsData[i].descriptors[m_swapchain->CurrentFrame()], 0, nullptr);
            vkCmdDrawIndexed(commandBuffer, m_indices.size(), 1, 0, 0, 0);
        }

        vkCmdEndRenderPass(commandBuffer);
        // End Render Pass
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
                m_swapchain = std::make_unique<VulkanSwapChain>(*m_device, *m_allocator,oldSwapChain);
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

                
        m_objectsData.resize(1000);
        glm::mat4 model = glm::mat4(1);
        for(size_t i =0;i<m_objectsData.size();i++){
            glm::mat4 transform = glm::translate(model, glm::vec3(2.0f* i,0.0f,0.0f));
            m_objectsData[i].transform =transform; 
        }
    }

    void Application::CreateVertexBuffer()
    {

        m_vertexBuffer =  std::make_unique<VulkanBuffer>(*m_device, *m_allocator, VulkanBufferType::VERTEX_BUFFER, m_vertices.data(), sizeof(Vertex)* m_vertices.size());
    }
    void Application::CreateIndexBuffer()
    {

        m_indexBuffer = std::make_unique<VulkanBuffer>(*m_device, *m_allocator, VulkanBufferType::INDEX_BUFFER, m_indices.data(), sizeof(u_int32_t) * m_indices.size());
    }

    void Application::CreateUniformBuffer()
    {


        for(auto i = 0; i< m_swapchain->MAX_FRAMES_IN_FLIGHT;i++){

            m_cameraData.cameraUBOs[i] = std::make_unique<VulkanBuffer>(*m_device, *m_allocator, VulkanBufferType::UNIFORM_BUFFER, sizeof(CameraUBO::Values));
        }


    for(size_t i =0; i<2;i++){
        for(size_t j =0; j<m_objectsData.size();j++){
            VkDeviceSize bufferSize = sizeof(ModelUBO::Values);
            VkMemoryAllocateInfo allocInfo = {};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.pNext = nullptr;
            allocInfo.allocationSize = 0;
            allocInfo.memoryTypeIndex = 0;
            VkBufferCreateInfo bufferInfo = {};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            bufferInfo.size = bufferSize;
            m_objectsData[j].modelUBOs[i].buffer.allocationHandle = m_allocator->AllocateBuffer(&bufferInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, &m_objectsData[j].modelUBOs[i].buffer.bufferHandle);
            m_objectsData[j].modelUBOs[i].buffer.data = m_allocator->MapMemory(m_objectsData[j].modelUBOs[i].buffer.allocationHandle);
        }
    }
    }

    void Application::UpdateUniformBuffer(uint32_t currentFrame)
    {
        //Camera
        CameraUBO::Values cameraUboValues{};
        cameraUboValues.proj = m_camera->Projection();
        cameraUboValues.view = m_camera->View();
        m_cameraData.cameraUBOs[currentFrame]->UpdateMemory(&cameraUboValues);
    
        for(size_t i =0; i<m_objectsData.size();i++){
            //m_objectsData[i].modelUBOs[currentFrame].values
            ModelUBO::Values temp;
            temp.model = m_objectsData[i].transform;
            memcpy(m_objectsData[i].modelUBOs[currentFrame].buffer.data, &temp, sizeof(ModelUBO::Values));
        }   
    }
    void Application::CreateDescriptorPool()
    {
        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = 2 + (m_objectsData.size() * 2);

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.maxSets = 2 + (m_objectsData.size()*2);

        VK_CHECK_RESULT(vkCreateDescriptorPool(m_device->LogicalDeviceHandle(), &poolInfo, nullptr, &m_descriptorPool));
    }
    void Application::CreateDescriptorSets()
    {

        std::vector<VkDescriptorSetLayout> layouts{m_defaultPipelineSpec.descriptorSetLayouts[0], m_defaultPipelineSpec.descriptorSetLayouts[1]};

        for(size_t i = 0;i<2;i++)
        {
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = m_descriptorPool;
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts = &layouts[0];
            vkAllocateDescriptorSets(m_device->LogicalDeviceHandle(), &allocInfo, &m_cameraData.descriptors[i]);
        }

        for (size_t i = 0; i < 2; i++){
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = m_cameraData.cameraUBOs[i]->BufferHandle();
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(CameraUBO::Values);    
            
            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet =  m_cameraData.descriptors[i];
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &bufferInfo;
            vkUpdateDescriptorSets(m_device->LogicalDeviceHandle(), 1, &descriptorWrite, 0, nullptr);
        }


        for (size_t i = 0; i < m_swapchain->MAX_FRAMES_IN_FLIGHT; i++)
        {
            for (size_t j = 0; j < m_objectsData.size(); j++)
            {

                VkDescriptorSetAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.descriptorPool = m_descriptorPool;
                allocInfo.descriptorSetCount = 1;
                allocInfo.pSetLayouts = &layouts[1];
                vkAllocateDescriptorSets(m_device->LogicalDeviceHandle(), &allocInfo, &m_objectsData[j].descriptors[i]);
            }
        }



        for (size_t i = 0; i < m_swapchain->MAX_FRAMES_IN_FLIGHT; i++)
        {
            for (size_t j = 0; j < m_objectsData.size(); j++)
            {

                VkDescriptorBufferInfo bufferInfo{};
                bufferInfo.buffer = m_objectsData[j].modelUBOs[i].buffer.bufferHandle;
                bufferInfo.offset = 0;
                bufferInfo.range = sizeof(ModelUBO::Values);

                VkWriteDescriptorSet descriptorWrite{};
                descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrite.dstSet = m_objectsData[j].descriptors[i];
                descriptorWrite.dstBinding = 0;
                descriptorWrite.dstArrayElement = 0;
                descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                descriptorWrite.descriptorCount = 1;
                descriptorWrite.pBufferInfo = &bufferInfo;
                vkUpdateDescriptorSets(m_device->LogicalDeviceHandle(), 1, &descriptorWrite, 0, nullptr);
            }
        }
    }

    void Application::CreateComputePipelineLayout(){

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

    void Application::CreateComputeStorageBuffers(){
        VkDeviceSize bufferSize = sizeof(int) *10;
        
        
        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.pNext = nullptr;
        allocInfo.allocationSize = 0;
        allocInfo.memoryTypeIndex = 0;
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        bufferInfo.size = bufferSize;

        m_ssbObjects[0].buffer.allocationHandle = m_allocator->AllocateBuffer(&bufferInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, &m_ssbObjects[0].buffer.bufferHandle);
        m_ssbObjects[0].buffer.data = m_allocator->MapMemory(m_ssbObjects[0].buffer.allocationHandle);

        m_ssbObjects[1].buffer.allocationHandle = m_allocator->AllocateBuffer(&bufferInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, &m_ssbObjects[1].buffer.bufferHandle);
        m_ssbObjects[1].buffer.data = m_allocator->MapMemory(m_ssbObjects[1].buffer.allocationHandle);
    }

    void Application::CreateComputeDescriptorPool(){
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

        for(int i=0;i<2;i++){
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
    void Application::CreateComputeCommandPool(){

        VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
        commandBufferAllocateInfo.sType =
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateInfo.commandPool = m_device->CommandPoolHandle();
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocateInfo.commandBufferCount = 1;

        VK_CHECK_RESULT(vkAllocateCommandBuffers(m_device->LogicalDeviceHandle(), &commandBufferAllocateInfo, &m_computeCommandBuffer));
    }

    void Application::BuildComputeCommands(){
        // Compute command buffer
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(m_computeCommandBuffer, &beginInfo);
        vkCmdBindPipeline(m_computeCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_computePipeline);
        vkCmdBindDescriptorSets(m_computeCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipelineLayoutCompute, 0, 1, &m_ssbObjects[0].descriptor, 0, nullptr);
        vkCmdBindDescriptorSets(m_computeCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipelineLayoutCompute, 1, 1, &m_ssbObjects[1].descriptor, 0, nullptr);
        vkCmdDispatch(m_computeCommandBuffer, 10, 1, 1);                                                                                          
        if (vkEndCommandBuffer(m_computeCommandBuffer) != VK_SUCCESS)                                                                              
        {
            APP_ERROR("failed to end command buffer");
        }

        
        std::vector<int> temp;
        temp.resize(10);
        int index =1;
        for(auto& it: temp){
            it= index++;
        }
        memcpy(m_ssbObjects[0].buffer.data, temp.data(), sizeof(int) * 10);

        for(auto& it: temp){
            it =0;
        }
        memcpy(m_ssbObjects[1].buffer.data, temp.data(), sizeof(int) * 10);
    }

    void Application::CalculateCompute(){
        {
        Timer time(true);
        VkSubmitInfo submitInfo2 = {};
        submitInfo2.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo2.commandBufferCount = 1;
        submitInfo2.pCommandBuffers = &m_computeCommandBuffer;

        vkQueueSubmit(m_device->GraphicsQueueHandle(), 1, &submitInfo2, VK_NULL_HANDLE); 
        vkQueueWaitIdle(m_device->GraphicsQueueHandle());
        }
        int* it =(int*)m_ssbObjects[0].buffer.data;
        for(int i=0;i<10;i++){
            APP_ERROR("{} ", *it);
            it= it+1;
        }
        APP_ERROR("-----------RESULT-------------");
        int* it2 =(int*)m_ssbObjects[1].buffer.data;
        for(int i=0;i<10;i++){
            APP_WARN("{} ", *it2);
            it2 = it2 + 1;
        }
        
    }
    void Application::TestingAbstraction(){
     APP_ERROR("TESTSING");
     
    }
}
