#include "VulkanPipeline.h"


namespace Pathfinding
{
    VulkanPipeline::VulkanPipeline(
        VulkanDevice &device,
        const std::string &vertFilepath,
        const std::string &fragFilepath,
        PipelineSpecification &pipelineSpecification)
        : m_deviceRef{device}
    {
        CreateGraphicsPipeline(vertFilepath, fragFilepath, pipelineSpecification);
        
    }

    VulkanPipeline::~VulkanPipeline()
    {
        vkDestroyPipeline(m_deviceRef.LogicalDeviceHandle(), m_pipelineHandle, nullptr);
        vkDestroyPipelineLayout(m_deviceRef.LogicalDeviceHandle(), m_pipelineLayoutHandle, nullptr);
    }

    std::vector<char> VulkanPipeline::ReadShaderFile(const std::string &path)
    {
        std::vector<char> shaderSource;
        std::ifstream shaderFile(path, std::ios::in | std::ios::binary);
        if (shaderFile)
        {
            shaderFile.seekg(0, std::ios::end);
            shaderSource.resize(shaderFile.tellg());
            shaderFile.seekg(0, std::ios::beg);
            shaderFile.read(shaderSource.data(), shaderSource.size());
            shaderFile.close();
        }
        else
        {
            CHECK_ERROR(APP_ERROR_VALUE, APP_ERROR("Failed to load shader!"));
        }
        return shaderSource;
    }

    void VulkanPipeline::CreateGraphicsPipeline(
        const std::string &vetexShaderPath,
        const std::string &fragmentShaderPath,
        PipelineSpecification &pipelineSpecification)
    {

        auto vertCode = ReadShaderFile(vetexShaderPath);
        auto fragCode = ReadShaderFile(fragmentShaderPath);

        CreateShaderModule(vertCode, &m_vertShaderModule);
        CreateShaderModule(fragCode, &m_fragShaderModule);

        VkPipelineShaderStageCreateInfo shaderStages[2];
        shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStages[0].module = m_vertShaderModule;
        shaderStages[0].pName = "main";
        shaderStages[0].flags = 0;
        shaderStages[0].pNext = nullptr;
        shaderStages[0].pSpecializationInfo = nullptr;
        shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages[1].module = m_fragShaderModule;
        shaderStages[1].pName = "main";
        shaderStages[1].flags = 0;
        shaderStages[1].pNext = nullptr;
        shaderStages[1].pSpecializationInfo = nullptr;

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = pipelineSpecification.vertexInputDescription.bindings.size();
        vertexInputInfo.pVertexBindingDescriptions = pipelineSpecification.vertexInputDescription.bindings.data();

        vertexInputInfo.vertexAttributeDescriptionCount = pipelineSpecification.vertexInputDescription.attributes.size();
        vertexInputInfo.pVertexAttributeDescriptions = pipelineSpecification.vertexInputDescription.attributes.data();
        
        
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        
    
        if (pipelineSpecification.pushConstantData){
            
            VkPushConstantRange pushConstantRange;
            pushConstantRange.offset = 0;
            pushConstantRange.size = sizeof(PipelinePushConstantData);
            pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo.setLayoutCount = 0;
            pipelineLayoutInfo.pushConstantRangeCount = 1;
            pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        }else{

            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo.setLayoutCount = 0;
            pipelineLayoutInfo.pushConstantRangeCount = 0;
        }
        
        //Based on descriptors to let know pipline what to expect what types and how many

        if(!pipelineSpecification.descriptorSetLayouts.empty()){
            pipelineLayoutInfo.pSetLayouts = pipelineSpecification.descriptorSetLayouts.data();
            pipelineLayoutInfo.setLayoutCount = pipelineSpecification.descriptorSetLayouts.size();
        }else{
            APP_ERROR("HEHEH CHECK!");
        }

        if (vkCreatePipelineLayout(m_deviceRef.LogicalDeviceHandle(), &pipelineLayoutInfo, nullptr, &m_pipelineLayoutHandle))
        {
            CHECK_ERROR(APP_ERROR_VALUE, APP_ERROR("Failed to create Pipline Layout!"));
        }


        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &pipelineSpecification.inputAssemblyInfo;
        pipelineInfo.pViewportState = &pipelineSpecification.viewportInfo;
        pipelineInfo.pRasterizationState = &pipelineSpecification.rasterizationInfo;
        pipelineInfo.pMultisampleState = &pipelineSpecification.multisampleInfo;
        pipelineInfo.pColorBlendState = &pipelineSpecification.colorBlendInfo;
        pipelineInfo.pDepthStencilState = &pipelineSpecification.depthStencilInfo;
        pipelineInfo.pDynamicState = &pipelineSpecification.dynamicStateInfo;

        pipelineInfo.layout = m_pipelineLayoutHandle;
    
        pipelineInfo.renderPass = pipelineSpecification.renderPass;
        pipelineInfo.subpass = pipelineSpecification.subpass;

        pipelineInfo.basePipelineIndex = -1;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(
                m_deviceRef.LogicalDeviceHandle(),
                VK_NULL_HANDLE,
                1,
                &pipelineInfo,
                nullptr,
                &m_pipelineHandle) != VK_SUCCESS)
        {
            CHECK_ERROR(APP_ERROR_VALUE, APP_ERROR("Failed to create Graphics Pipline!"));
        }

        vkDestroyShaderModule(m_deviceRef.LogicalDeviceHandle(), m_vertShaderModule, nullptr);
        vkDestroyShaderModule(m_deviceRef.LogicalDeviceHandle(), m_fragShaderModule, nullptr);
    }

    void VulkanPipeline::CreateShaderModule(const std::vector<char> &code, VkShaderModule *shaderModule)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

        if (vkCreateShaderModule(m_deviceRef.LogicalDeviceHandle(),&createInfo, nullptr, shaderModule) != VK_SUCCESS)
        {
            CHECK_ERROR(APP_ERROR_VALUE, APP_ERROR("Failed to create Shader Module!"));
        }
    }

    void VulkanPipeline::Bind(VkCommandBuffer commandBuffer)
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineHandle);
    }

    PipelineSpecification VulkanPipeline::DefaultPipelineSpecification(VkRenderPass renderPass)
    {

        PipelineSpecification pipelineSpecification{};
        pipelineSpecification.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        pipelineSpecification.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        pipelineSpecification.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

       
        pipelineSpecification.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        pipelineSpecification.viewportInfo.viewportCount = 1;
        pipelineSpecification.viewportInfo.scissorCount = 1;

        pipelineSpecification.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        pipelineSpecification.rasterizationInfo.depthClampEnable = VK_FALSE;
        pipelineSpecification.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
        pipelineSpecification.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL; // whole objectVK_POLYGON_MODE_LINE;
        pipelineSpecification.rasterizationInfo.lineWidth = 1.0f;
        pipelineSpecification.rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
        pipelineSpecification.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
        pipelineSpecification.rasterizationInfo.depthBiasEnable = VK_FALSE;

      
        pipelineSpecification.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        pipelineSpecification.multisampleInfo.sampleShadingEnable = VK_FALSE;
        pipelineSpecification.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        pipelineSpecification.colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        pipelineSpecification.colorBlendAttachment.blendEnable = VK_FALSE;

        
        pipelineSpecification.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        pipelineSpecification.colorBlendInfo.logicOpEnable = VK_FALSE;
        pipelineSpecification.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;
        pipelineSpecification.colorBlendInfo.attachmentCount = 1;
        pipelineSpecification.colorBlendInfo.pAttachments = &pipelineSpecification.colorBlendAttachment;
        pipelineSpecification.colorBlendInfo.blendConstants[0] = 0.0f;
        pipelineSpecification.colorBlendInfo.blendConstants[1] = 0.0f;
        pipelineSpecification.colorBlendInfo.blendConstants[2] = 0.0f;
        pipelineSpecification.colorBlendInfo.blendConstants[3] = 0.0f;

        pipelineSpecification.dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        pipelineSpecification.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        pipelineSpecification.dynamicStateInfo.pDynamicStates = pipelineSpecification.dynamicStateEnables.data();
        pipelineSpecification.dynamicStateInfo.dynamicStateCount =
            static_cast<uint32_t>(pipelineSpecification.dynamicStateEnables.size());
        pipelineSpecification.dynamicStateInfo.flags = 0;


        pipelineSpecification.renderPass = renderPass;

        return pipelineSpecification;
    }
} 
