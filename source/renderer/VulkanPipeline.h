#ifndef _RENDERER_VULKAN_PIPELINE_H_
#define _RENDERER_VULKAN_PIPELINE_H_

#include "VulkanDevice.h"
#include "../core/Logger.h"
#include <fstream>

namespace VulkanPathfinding{
struct PipelineConfigInfo
{
    PipelineConfigInfo() = default;
    PipelineConfigInfo(const PipelineConfigInfo &) = delete;
    PipelineConfigInfo &operator=(const PipelineConfigInfo &) = delete;

    std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
    VkPipelineViewportStateCreateInfo viewportInfo;
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    VkPipelineRasterizationStateCreateInfo rasterizationInfo;
    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    VkPipelineColorBlendStateCreateInfo colorBlendInfo;
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
    std::vector<VkDynamicState> dynamicStateEnables;
    VkPipelineDynamicStateCreateInfo dynamicStateInfo;
    VkPipelineLayout pipelineLayout = nullptr;
    VkRenderPass renderPass = nullptr;
    uint32_t subpass = 0;
};

class VulkanPipeline
{
public:
    VulkanPipeline(
        VulkanDevice &device,
        const std::string &vertFilepath,
        const std::string &fragFilepath,
        PipelineConfigInfo &configInfo);
    ~VulkanPipeline();

    VulkanPipeline(const VulkanPipeline &) = delete;
    VulkanPipeline &operator=(const VulkanPipeline &) = delete;

    void Bind(VkCommandBuffer commandBuffer);

    static void DefaultPipelineConfigInfo(PipelineConfigInfo &configInfo);

private:
    static std::vector<char> ReadShaderFile(const std::string &path);

    void CreateGraphicsPipeline(
        const std::string &vertFilepath,
        const std::string &fragFilepath,
        PipelineConfigInfo &configInfo);

    void CreateShaderModule(const std::vector<char> &code, VkShaderModule *shaderModule);

    VulkanDevice &m_deviceRef;
    VkPipeline m_graphicsPipeline;
    VkShaderModule m_vertShaderModule;
    VkShaderModule m_fragShaderModule;
};
}
#endif