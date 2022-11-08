#ifndef _RENDERER_VULKAN_PIPELINE_H_
#define _RENDERER_VULKAN_PIPELINE_H_

#include <fstream>


#include "VulkanDevice.h"
#include "../core/Logger.h"
#include "../objects/Object.h"

namespace Pathfinding{

    struct VertexInputDescription
    {

        std::vector<VkVertexInputBindingDescription> bindings;
        std::vector<VkVertexInputAttributeDescription> attributes;

        static VertexInputDescription GetVertexDescription()
        {
            VertexInputDescription description;

            VkVertexInputBindingDescription mainBinding = {};
            mainBinding.binding = 0;
            mainBinding.stride = sizeof(Vertex);
            mainBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            description.bindings.push_back(mainBinding);

            VkVertexInputAttributeDescription positionAttribute = {};
            positionAttribute.binding = 0;
            positionAttribute.location = 0;
            positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
            positionAttribute.offset = offsetof(Vertex, position);

            VkVertexInputAttributeDescription colorAttribute = {};
            colorAttribute.binding = 0;
            colorAttribute.location = 1;
            colorAttribute.format = VK_FORMAT_R32G32B32A32_SFLOAT;
            colorAttribute.offset = offsetof(Vertex, color);

            description.attributes.push_back(positionAttribute);
            description.attributes.push_back(colorAttribute);
            return description;
        }
    };

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