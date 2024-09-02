#pragma once

#include <vulkan/vulkan.h>

#include <span>
#include <vector>

class PipelineBuilder {
 public:
  std::vector<VkPipelineShaderStageCreateInfo> _shaderStages;

  VkPipelineVertexInputStateCreateInfo _vertexInputInfo;
  VkPipelineInputAssemblyStateCreateInfo _inputAssembly;
  VkPipelineRasterizationStateCreateInfo _rasterizer;
  VkPipelineColorBlendAttachmentState _colorBlendAttachment;
  VkPipelineMultisampleStateCreateInfo _multisampling;
  VkPipelineLayout _pipelineLayout;
  VkPipelineDepthStencilStateCreateInfo _depthStencil;
  VkPipelineRenderingCreateInfo _renderInfo;
  VkFormat _colorAttachmentformat;

  PipelineBuilder();

  void clear();

  VkPipeline build_pipeline(VkDevice device);
  void set_shaders(VkShaderModule vertexShader, VkShaderModule fragmentShader);
  void set_input_topology(VkPrimitiveTopology topology);
  void set_polygon_mode(VkPolygonMode mode);
  void set_cull_mode(VkCullModeFlags cullMode, VkFrontFace frontFace);
  void set_multisampling_none();
  void disable_blending();
  void enable_blending_additive();
  void enable_blending_alphablend();

  void vertex_input(
      std::span<VkVertexInputBindingDescription> bindings,
      std::span<const VkVertexInputAttributeDescription> attributes);
  void set_color_attachment_format(VkFormat format);
  void set_depth_format(VkFormat format);
  void disable_depthtest();
  void enable_depthtest(bool depthWriteEnable, VkCompareOp op);
};
