#pragma once

#include <vulkan/vulkan.h>

#include <expected>
#include <glm/glm.hpp>
#include <optional>
#include <string_view>
#include <vector>

#include "struct.hpp"

constexpr std::string_view VIKING_MODEL = "models/viking_room.obj";
constexpr std::string_view VIKING_TEXTURE = "textures/viking_room.png";

class VikingRoom {
 public:
  VikingRoom();
  VikingRoom(const VikingRoom &) = delete;
  VikingRoom(VikingRoom &&) = delete;
  VikingRoom &operator=(const VikingRoom &) = delete;
  VikingRoom &operator=(VikingRoom &&) = delete;
  ~VikingRoom();

  void load_model();
  void build_pipeline();
  void draw(VkCommandBuffer cmd);
  void init_data();

 private:
  struct PushConstants {
    glm::mat4 mvp;
    glm::vec3 col;
  };

  std::vector<Vertex> _vertexData;
  std::vector<uint32_t> _indexData;

  VkPipeline _pipeline{};
  VkPipelineLayout _pipelineLayout{};
  VkDeviceAddress _vertexBufferAddress{};
  std::optional<GPUMeshBuffers> _meshBuffers;
};
