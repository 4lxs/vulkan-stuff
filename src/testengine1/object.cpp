#include "object.hpp"

#include <fmt/format.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include <cstddef>
#include <expected>
#include <glm/gtc/matrix_transform.hpp>

#include "engine.hpp"
#include "helpers.hpp"
#include "struct.hpp"
#include "vulkan/pipelinebuilder.hpp"
#include "vulkan/util.hpp"

TriangleObject::~TriangleObject() {
  Engine& engine = Engine::instance();

  vkDestroyPipelineLayout(engine._device, _pipelineLayout, nullptr);
  vkDestroyPipeline(engine._device, _pipeline, nullptr);
}

void TriangleObject::build_pipeline() {
  Engine& engine = Engine::instance();

  VkShaderModule triangleFragShader{};
  if (!vkutil::load_shader_module("shaders/colored_triangle.frag.spv",
                                  engine._device, &triangleFragShader)) {
    throw std::runtime_error(
        "Error when building the triangle fragment shader module");
  }

  fmt::print("Triangle fragment shader succesfully loaded\n");

  VkShaderModule triangleVertexShader{};
  if (!vkutil::load_shader_module("shaders/colored_triangle.vert.spv",
                                  engine._device, &triangleVertexShader)) {
    throw std::runtime_error(
        "Error when building the triangle fragment shader module");
  }
  fmt::print("Triangle vertex shader succesfully loaded");

  VkVertexInputBindingDescription bindingDescription{};
  bindingDescription.binding = 0;
  bindingDescription.stride = sizeof(Vertex);
  bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
  attributeDescriptions[0].binding = 0;
  attributeDescriptions[0].location = 0;
  attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
  attributeDescriptions[0].offset = offsetof(Vertex, position);
  attributeDescriptions[1].binding = 0;
  attributeDescriptions[1].location = 2;
  attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
  attributeDescriptions[1].offset = offsetof(Vertex, color);
  attributeDescriptions[2].binding = 0;
  attributeDescriptions[2].location = 4;
  attributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
  attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

  VkPushConstantRange pushConstant{};
  pushConstant.offset = 0;
  pushConstant.size = sizeof(PushConstants);
  pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  VkPipelineLayoutCreateInfo pipeline_layout_info{};
  pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_layout_info.pNext = nullptr;
  pipeline_layout_info.pPushConstantRanges = &pushConstant;
  pipeline_layout_info.pushConstantRangeCount = 1;
  pipeline_layout_info.pSetLayouts = &engine._descriptorSetLayout;
  pipeline_layout_info.setLayoutCount = 1;

  vk_check(vkCreatePipelineLayout(engine._device, &pipeline_layout_info,
                                  nullptr, &_pipelineLayout));

  PipelineBuilder pipelineBuilder;
  pipelineBuilder._pipelineLayout = _pipelineLayout;
  pipelineBuilder.set_shaders(triangleVertexShader, triangleFragShader);
  pipelineBuilder.set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
  pipelineBuilder.set_polygon_mode(VK_POLYGON_MODE_FILL);
  pipelineBuilder.set_cull_mode(VK_CULL_MODE_BACK_BIT,
                                VK_FRONT_FACE_COUNTER_CLOCKWISE);
  pipelineBuilder.set_multisampling_none();
  pipelineBuilder.disable_blending();
  pipelineBuilder.disable_depthtest();
  pipelineBuilder.vertex_input(std::span(&bindingDescription, 1),
                               attributeDescriptions);

  // connect the image format we will draw into, from draw image
  pipelineBuilder.set_color_attachment_format(engine._drawImage.format);
  pipelineBuilder.set_depth_format(VK_FORMAT_UNDEFINED);

  _pipeline = pipelineBuilder.build_pipeline(engine._device);

  vkDestroyShaderModule(engine._device, triangleFragShader, nullptr);
  vkDestroyShaderModule(engine._device, triangleVertexShader, nullptr);
}

void TriangleObject::draw(VkCommandBuffer cmd) {
  Engine& engine = Engine::instance();
  FrameData& frame = engine.get_current_frame();

  vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);

  glm::mat4 Projection = glm::perspective(
      glm::radians(45.0F),
      float(engine._drawExtent.width) / float(engine._drawExtent.height), 0.1F,
      10.0F);

  glm::mat4 View =
      glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                  glm::vec3(0.0f, 0.0f, 1.0f));

  glm::mat4 Model{glm::rotate(glm::mat4(1.0F), glm::radians(90.0F),
                              glm::vec3(0.0F, 0.0F, 1.0F))};

  glm::mat4 mvp = Projection * View * Model;

  PushConstants pushConstants;
  pushConstants.mvp = mvp;
  pushConstants.col = glm::vec3(1., 0., 0.);

  vkCmdPushConstants(cmd, _pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                     sizeof(PushConstants), &pushConstants);

  std::array<VkBuffer, 1> vertexBuffers{_meshBuffers->vertexBuffer._buffer};
  std::array<VkDeviceSize, 1> offsets{0};
  vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers.data(), offsets.data());

  vkCmdBindIndexBuffer(cmd, _meshBuffers->indexBuffer._buffer, 0,
                       VK_INDEX_TYPE_UINT16);

  vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout,
                          0, 1, &frame._descriptorSet, 0, nullptr);

  vkCmdDrawIndexed(cmd, _indexData.size(), 1, 0, 0, 0);
}

void TriangleObject::init_data() {
  Engine& engine = Engine::instance();
  _meshBuffers.emplace(std::move(engine.upload_mesh(_vertexData, _indexData)));
}
