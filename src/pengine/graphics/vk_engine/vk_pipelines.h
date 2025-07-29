#pragma once

#include "vk_types.h"

namespace penguin_engine {
namespace graphics {
namespace vulkan {
    class PipelineBuilder {
    public:
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

        VkPipelineInputAssemblyStateCreateInfo  inputAssembly;
        VkPipelineRasterizationStateCreateInfo  rasterizer;
        VkPipelineColorBlendAttachmentState     colorBlendAttachment;
        VkPipelineMultisampleStateCreateInfo    multisampling;
        VkPipelineLayout                        pipelineLayout;
        VkPipelineDepthStencilStateCreateInfo   depthStencil;
        VkPipelineRenderingCreateInfo           renderInfo;
        VkFormat                                colorAttachmentformat;

        PipelineBuilder() { clear(); }

        VkPipeline build_pipeline(VkDevice device);

        void set_shaders(VkShaderModule vertexShader, VkShaderModule fragmentShader);

        void set_input_topology(VkPrimitiveTopology topology);

        void set_polygon_mode(VkPolygonMode mode);

        void set_cull_mode(VkCullModeFlags cullMode, VkFrontFace frontFace);

        void set_multisampling_none();

		void disable_blending();

		void set_color_attachment_format(VkFormat format);

		void set_depth_format(VkFormat format);

        void enable_depthtest(bool depthWriteEnable, VkCompareOp op);

		void disable_depthtest();

        void clear();
    };
}
}
}

