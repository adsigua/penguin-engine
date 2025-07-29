#include "vk_pipelines.h"
#include "vk_engine.h"
#include "vk_initializers.h"
#include "vk_utils.h"

namespace penguin_engine {
namespace graphics {
namespace vulkan {

	void VKEngine::init_pipelines()
	{
		init_background_pipeline();

		//init_triangle_pipeline();

		init_mesh_pipeline();
	}

	void VKEngine::init_background_pipeline()
	{
		VkPipelineLayoutCreateInfo computeLayout{};
		computeLayout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		computeLayout.pNext = nullptr;
		computeLayout.pSetLayouts = &_draw_image_data.drawImageDescriptorLayout;
		computeLayout.setLayoutCount = 1;

		VkPushConstantRange pushConstant{};
		pushConstant.offset = 0;
		pushConstant.size = sizeof(ComputePushConstants);
		pushConstant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

		computeLayout.pPushConstantRanges = &pushConstant;
		computeLayout.pushConstantRangeCount = 1;

		VK_CHECK(vkCreatePipelineLayout(_logical_device, &computeLayout, nullptr, &_background_pipeline_layout));

		VkShaderModule gradientShader;
		if (!vkutil::load_shader_module("gradient_color.comp.spv", _logical_device, &gradientShader)) {
			printf("Error when building the compute shader \n");
		}

		VkShaderModule skyShader;
		if (!vkutil::load_shader_module("sky.comp.spv", _logical_device, &skyShader)) {
			printf("Error when building the compute shader \n");
		}

		VkPipelineShaderStageCreateInfo stageinfo{};
		stageinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stageinfo.pNext = nullptr;
		stageinfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		stageinfo.module = gradientShader;
		stageinfo.pName = "main";

		VkComputePipelineCreateInfo computePipelineCreateInfo{};
		computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		computePipelineCreateInfo.pNext = nullptr;
		computePipelineCreateInfo.layout = _background_pipeline_layout;
		computePipelineCreateInfo.stage = stageinfo;

		ComputeEffect gradient;
		gradient.pipeline_layout = _background_pipeline_layout;
		gradient.name = "gradient";
		gradient.data = {};

		//default colors
		gradient.data.data1 = glm::vec4(1, 0, 0, 1);
		gradient.data.data2 = glm::vec4(0, 0, 1, 1);

		VK_CHECK(vkCreateComputePipelines(_logical_device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &gradient.pipeline));

		//change the shader module only to create the sky shader
		computePipelineCreateInfo.stage.module = skyShader;

		ComputeEffect sky;
		sky.pipeline_layout = _background_pipeline_layout;
		sky.name = "sky";
		sky.data = {};
		//default sky parameters
		sky.data.data1 = glm::vec4(0.1, 0.2, 0.4, 0.97);

		VK_CHECK(vkCreateComputePipelines(_logical_device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &sky.pipeline));

		//add the 2 background effects into the array
		_backgroundEffects.push_back(gradient);
		_backgroundEffects.push_back(sky);

		//destroy structures properly
		vkDestroyShaderModule(_logical_device, gradientShader, nullptr);
		vkDestroyShaderModule(_logical_device, skyShader, nullptr);
	}

	void VKEngine::init_triangle_pipeline()
	{
		VkShaderModule triangleFragShader;
		if (!vkutil::load_shader_module("colored_triangle.frag.spv", _logical_device, &triangleFragShader)) {
			printf("Error when building the triangle fragment shader module.\n");
		}
		else {
			printf("Triangle fragment shader succesfully loaded.\n");
		}

		VkShaderModule triangleVertexShader;
		if (!vkutil::load_shader_module("colored_triangle.vert.spv", _logical_device, &triangleVertexShader)) {
			printf("Error when building the triangle vertex shader module.\n");
		}
		else {
			printf("Triangle vertex shader succesfully loaded.\n");
		}

		//build the pipeline layout that controls the inputs/outputs of the shader
		//we are not using descriptor sets or other systems yet, so no need to use anything other than empty default
		VkPipelineLayoutCreateInfo pipeline_layout_info = vkinit::pipeline_layout_create_info();

		VK_CHECK(vkCreatePipelineLayout(_logical_device, &pipeline_layout_info, nullptr, &_triangle_pipeline_layout));

		PipelineBuilder pipelineBuilder;

		//use the triangle layout we created
		pipelineBuilder.pipelineLayout = _triangle_pipeline_layout;
		//connecting the vertex and pixel shaders to the pipeline
		pipelineBuilder.set_shaders(triangleVertexShader, triangleFragShader);
		//it will draw triangles
		pipelineBuilder.set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
		//filled triangles
		pipelineBuilder.set_polygon_mode(VK_POLYGON_MODE_FILL);
		//no backface culling
		pipelineBuilder.set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
		//no multisampling
		pipelineBuilder.set_multisampling_none();
		//no blending
		pipelineBuilder.disable_blending();
		//no depth testing
		pipelineBuilder.disable_depthtest();

		//connect the image format we will draw into, from draw image
		pipelineBuilder.set_color_attachment_format(_draw_image_data.allocatedImage.imageFormat);
		pipelineBuilder.set_depth_format(VK_FORMAT_UNDEFINED);

		//finally build the pipeline
		_triangle_pipeline = pipelineBuilder.build_pipeline(_logical_device);

		//clean structures
		vkDestroyShaderModule(_logical_device, triangleFragShader, nullptr);
		vkDestroyShaderModule(_logical_device, triangleVertexShader, nullptr);
	}
	
	void VKEngine::init_mesh_pipeline()
	{
		VkShaderModule triangleFragShader;
		if (!vkutil::load_shader_module("colored_triangle.frag.spv", _logical_device, &triangleFragShader)) {
			printf("Error when building the triangle fragment shader module.\n");
		}
		else {
			printf("Triangle fragment shader succesfully loaded.\n");
		}

		VkShaderModule triangleVertexShader;
		if (!vkutil::load_shader_module("colored_triangle.vert.spv", _logical_device, &triangleVertexShader)) {
			printf("Error when building the triangle vertex shader module.\n");
		}
		else {
			printf("Triangle vertex shader succesfully loaded.\n");
		}

		VkPushConstantRange bufferRange{};
		bufferRange.offset = 0;
		bufferRange.size = sizeof(GPUDrawPushConstants);
		bufferRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkPipelineLayoutCreateInfo pipeline_layout_info = vkinit::pipeline_layout_create_info();
		pipeline_layout_info.pPushConstantRanges = &bufferRange;
		pipeline_layout_info.pushConstantRangeCount = 1;
		VK_CHECK(vkCreatePipelineLayout(_logical_device, &pipeline_layout_info, nullptr, &_mesh_pipeline_layout));

		PipelineBuilder pipelineBuilder;

		//use the triangle layout we created
		pipelineBuilder.pipelineLayout = _mesh_pipeline_layout;
		//connecting the vertex and pixel shaders to the pipeline
		pipelineBuilder.set_shaders(triangleVertexShader, triangleFragShader);
		//it will draw triangles
		pipelineBuilder.set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
		//filled triangles
		pipelineBuilder.set_polygon_mode(VK_POLYGON_MODE_FILL);
		//no backface culling
		pipelineBuilder.set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
		//no multisampling
		pipelineBuilder.set_multisampling_none();
		//no blending
		pipelineBuilder.disable_blending();
		//no depth testing
		pipelineBuilder.enable_depthtest(true, VK_COMPARE_OP_GREATER_OR_EQUAL);

		//connect the image format we will draw into, from draw image
		pipelineBuilder.set_color_attachment_format(_draw_image_data.allocatedImage.imageFormat);
		pipelineBuilder.set_depth_format(_depth_image.imageFormat);

		//finally build the pipeline
		_mesh_pipeline = pipelineBuilder.build_pipeline(_logical_device);

		//clean structures
		vkDestroyShaderModule(_logical_device, triangleFragShader, nullptr);
		vkDestroyShaderModule(_logical_device, triangleVertexShader, nullptr);
	}

	VkPipeline PipelineBuilder::build_pipeline(VkDevice device)
	{
		// make viewport state from our stored viewport and scissor.
		// at the moment we wont support multiple viewports or scissors
		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.pNext = nullptr;

		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		// setup dummy color blending. We arent using transparent objects yet
		// the blending is just "no blend", but we do write to the color attachment
		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.pNext = nullptr;

		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;

		// completely clear VertexInputStateCreateInfo, as we have no need for it
		VkPipelineVertexInputStateCreateInfo vertexInputInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };

		// build the actual pipeline
		// we now use all of the info structs we have been writing into into this one
		// to create the pipeline
		VkGraphicsPipelineCreateInfo pipelineInfo = { .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
		// connect the renderInfo to the pNext extension mechanism
		pipelineInfo.pNext = &renderInfo;

		pipelineInfo.stageCount = (uint32_t)shaderStages.size();
		pipelineInfo.pStages = shaderStages.data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.layout = pipelineLayout;

		VkDynamicState state[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

		VkPipelineDynamicStateCreateInfo dynamicInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
		dynamicInfo.pDynamicStates = &state[0];
		dynamicInfo.dynamicStateCount = 2;

		pipelineInfo.pDynamicState = &dynamicInfo;

		// its easy to error out on create graphics pipeline, so we handle it a bit
		// better than the common VK_CHECK case
		VkPipeline newPipeline;
		if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo,
			nullptr, &newPipeline) != VK_SUCCESS) {
			printf("failed to create pipeline");
			return VK_NULL_HANDLE; // failed to create graphics pipeline
		}
		else {
			return newPipeline;
		}
	}

	void PipelineBuilder::set_shaders(VkShaderModule vertexShader, VkShaderModule fragmentShader)
	{
		shaderStages.clear();

		shaderStages.push_back(
			vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, vertexShader));

		shaderStages.push_back(
			vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, fragmentShader));
	}

	void PipelineBuilder::set_input_topology(VkPrimitiveTopology topology)
	{
		inputAssembly.topology = topology;
		// we are not going to use primitive restart on the entire tutorial so leave
		// it on false
		inputAssembly.primitiveRestartEnable = VK_FALSE;
	}

	void PipelineBuilder::set_polygon_mode(VkPolygonMode mode)
	{
		rasterizer.polygonMode = mode;
		rasterizer.lineWidth = 1.f;
	}

	void PipelineBuilder::set_cull_mode(VkCullModeFlags cullMode, VkFrontFace frontFace)
	{
		rasterizer.cullMode = cullMode;
		rasterizer.frontFace = frontFace;
	}

	void PipelineBuilder::set_multisampling_none()
	{
		multisampling.sampleShadingEnable = VK_FALSE;
		// multisampling defaulted to no multisampling (1 sample per pixel)
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f;
		multisampling.pSampleMask = nullptr;
		// no alpha to coverage either
		multisampling.alphaToCoverageEnable = VK_FALSE;
		multisampling.alphaToOneEnable = VK_FALSE;
	}

	void PipelineBuilder::disable_blending()
	{
		// default write mask
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		// no blending
		colorBlendAttachment.blendEnable = VK_FALSE;
	}

	void PipelineBuilder::set_color_attachment_format(VkFormat format)
	{
		colorAttachmentformat = format;
		// connect the format to the renderInfo  structure
		renderInfo.colorAttachmentCount = 1;
		renderInfo.pColorAttachmentFormats = &colorAttachmentformat;
	}

	void PipelineBuilder::set_depth_format(VkFormat format)
	{
		renderInfo.depthAttachmentFormat = format;
	}

	void PipelineBuilder::enable_depthtest(bool depthWriteEnable, VkCompareOp op)
	{
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = depthWriteEnable;
		depthStencil.depthCompareOp = op;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;
		depthStencil.front = {};
		depthStencil.back = {};
		depthStencil.minDepthBounds = 0.f;
		depthStencil.maxDepthBounds = 1.f;
	}

	void PipelineBuilder::disable_depthtest()
	{
		depthStencil.depthTestEnable = VK_FALSE;
		depthStencil.depthWriteEnable = VK_FALSE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_NEVER;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;
		depthStencil.front = {};
		depthStencil.back = {};
		depthStencil.minDepthBounds = 0.f;
		depthStencil.maxDepthBounds = 1.f;
	}

	void PipelineBuilder::clear()
	{
		// clear all of the structs we need back to 0 with their correct stype

		inputAssembly = { .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };

		rasterizer = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };

		colorBlendAttachment = {};

		multisampling = { .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };

		pipelineLayout = {};

		depthStencil = { .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };

		renderInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };

		shaderStages.clear();
	}
}
}
}
