#pragma once

#include <vulkan/vulkan.h>

#include <expected>
#include <glm/glm.hpp>
#include <memory>
#include <optional>
#include <vector>

#include "loadMesh.hpp"
#include "struct.hpp"

class MonkeyHead {
 public:
  MonkeyHead() = default;
  MonkeyHead(const MonkeyHead &) = delete;
  MonkeyHead(MonkeyHead &&) = delete;
  MonkeyHead &operator=(const MonkeyHead &) = delete;
  MonkeyHead &operator=(MonkeyHead &&) = delete;
  ~MonkeyHead();

  void load_model();
  void build_pipeline();
  void draw(VkCommandBuffer cmd);
  void init_data();

 private:
  struct PushConstants {
    glm::mat4 mvp;
    VkDeviceAddress vertexBuffer{};
  };

  std::vector<std::shared_ptr<MeshAsset>> _meshes;

  VkPipeline _pipeline{};
  VkPipelineLayout _pipelineLayout{};
};
