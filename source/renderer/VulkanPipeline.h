#ifndef _RENDERER_VULKAN_PIPELINE_H_
#define _RENDERER_VULKAN_PIPELINE_H_

#include <fstream>

#include "VulkanDevice.h"

#include "../core/Logger.h"
#include "../objects/Object.h"

namespace VulkanPathfinding{


    struct PipelinePushConstantData{
        alignas(16)glm::mat4 projection;
    };

    struct PipelineSpecification
    {
        std::unique_ptr<PipelinePushConstantData> pushConstantData;
        VertexInputDescription vertexInputDescription{};
        VkPipelineViewportStateCreateInfo viewportInfo;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisampleInfo;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
        std::vector<VkDynamicState> dynamicStateEnables;
        VkPipelineDynamicStateCreateInfo dynamicStateInfo;

        uint32_t subpass = 0;
        VkRenderPass renderPass = nullptr;
    };

class VulkanPipeline
{
public:
    VulkanPipeline(
        VulkanDevice &device,
        const std::string &vertFilepath,
        const std::string &fragFilepath,
        PipelineSpecification &pipelineData);
    ~VulkanPipeline();

    VulkanPipeline(const VulkanPipeline &) = delete;
    VulkanPipeline &operator=(const VulkanPipeline &) = delete;

    void Bind(VkCommandBuffer commandBuffer);
    
    VkPipelineLayout PipelineLayoutHandle(){return m_pipelineLayoutHandle;}
    static PipelineSpecification DefaultPipelineSpecification(VkRenderPass renderPass);

private:
    

    static std::vector<char> ReadShaderFile(const std::string &path);

    void CreateGraphicsPipeline(
        const std::string &vertFilepath,
        const std::string &fragFilepath,
        PipelineSpecification &pipelineData);

    void CreateShaderModule(const std::vector<char> &code, VkShaderModule *shaderModule);

    VulkanDevice &m_deviceRef;
    VkShaderModule m_vertShaderModule;
    VkShaderModule m_fragShaderModule;

    VkPipelineLayout m_pipelineLayoutHandle{VK_NULL_HANDLE};
    VkPipeline m_pipelineHandle{VK_NULL_HANDLE};
};
}
#endif