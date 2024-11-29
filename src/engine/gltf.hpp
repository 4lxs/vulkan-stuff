#pragma once

#include <vulkan/vulkan_core.h>
#include <memory>
#include <unordered_map>
#include <vector>
#include "struct.hpp"

namespace x::gltf {

struct MeshNode{};
struct MeshAsset{};
struct GLTFMaterial{};

enum class TextureId: uint32_t {};

// renderable object
class Scene {
 public:
  static std::unique_ptr<Scene> load(std::string_view filePath);

    std::unordered_map<std::string, std::shared_ptr<MeshAsset>> meshes;
    std::unordered_map<std::string, std::shared_ptr<MeshNode>> nodes;
    std::unordered_map<std::string, AllocatedImage> images;
    std::unordered_map<std::string, std::shared_ptr<GLTFMaterial>> materials;

    std::vector<VkSampler> samplers;
};

}  // namespace x::gltf
