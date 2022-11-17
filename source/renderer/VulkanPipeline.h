#ifndef _RENDERER_VULKAN_PIPELINE_H_
#define _RENDERER_VULKAN_PIPELINE_H_

#include <fstream>


#include "VulkanDevice.h"
#include "../core/Logger.h"
#include "../objects/Object.h"
#include "VulkanBuffer.h"
namespace Pathfinding{
    
    struct PipelinePushConstantData{
        alignas(16)glm::vec4 color;
    };

    struct PipelineSpecification
    {
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
        std::unique_ptr<PipelinePushConstantData> pushConstantData;
        VulkanVertexInputDescription vertexInputDescription;
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

        void AddDescriptorSetLayout(VulkanDevice &device)
        {

            
            
            {
            VkDescriptorSetLayout descriptorSetLayout;
            VkDescriptorSetLayoutBinding layoutBindings;
            layoutBindings.binding = 0;
            layoutBindings.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            layoutBindings.descriptorCount = 1;
            layoutBindings.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            
            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = 1;
            layoutInfo.pBindings = &layoutBindings;
            
            VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device.LogicalDeviceHandle(), &layoutInfo, nullptr, &descriptorSetLayout));

            descriptorSetLayouts.emplace_back(descriptorSetLayout);
            }

            {
                VkDescriptorSetLayout descriptorSetLayout;
                VkDescriptorSetLayoutBinding layoutBindings;
                layoutBindings.binding = 0;
                layoutBindings.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                layoutBindings.descriptorCount = 1;
                layoutBindings.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

                VkDescriptorSetLayoutCreateInfo layoutInfo{};
                layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                layoutInfo.bindingCount = 1;
                layoutInfo.pBindings = &layoutBindings;

                VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device.LogicalDeviceHandle(), &layoutInfo, nullptr, &descriptorSetLayout));

                descriptorSetLayouts.emplace_back(descriptorSetLayout);
            }
        }
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
    static std::vector<char> ReadShaderFile(const std::string &path);
private:
    


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