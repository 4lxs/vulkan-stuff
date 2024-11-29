#pragma once

#include <vk_mem_alloc.h>

#include <functional>
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>

struct AllocatedImage {
  AllocatedImage(vk::Format format, vk::Extent3D extent,
                 vk::ImageUsageFlags usages, VmaAllocationCreateInfo allocInfo,
                 vk::ImageAspectFlags imageAspect);
  VkImage image;
  VkImageView view;
  VkExtent3D extent;
  VkFormat format;
  VmaAllocation allocation;
};

class AllocatedBuffer {
 public:
  AllocatedBuffer(size_t allocSize, VkBufferUsageFlags usage,
                  VmaMemoryUsage memoryUsage);
  AllocatedBuffer(const AllocatedBuffer &) = delete;
  AllocatedBuffer(AllocatedBuffer &&) noexcept;
  AllocatedBuffer &operator=(const AllocatedBuffer &) = delete;
  AllocatedBuffer &operator=(AllocatedBuffer &&) = delete;
  ~AllocatedBuffer();

  VkBuffer _buffer{};
  VmaAllocation _allocation{};
  VmaAllocationInfo _info{};
};

struct GPUMeshBuffers {
  AllocatedBuffer indexBuffer;
  AllocatedBuffer vertexBuffer;
  VkDeviceAddress vertexBufferAddress;
};

struct Vertex {
  glm::vec3 position;
  // float uv_x;
  // glm::vec3 normal;
  // float uv_y;
  glm::vec4 color;
  glm::vec2 texCoord;

  bool operator==(const Vertex &other) const {
    return position == other.position && color == other.color &&
           texCoord == other.texCoord;
  }
};

template <>
struct std::hash<Vertex> {
  size_t operator()(Vertex const &vertex) const {
    return ((std::hash<glm::vec3>()(vertex.position) ^
             (std::hash<glm::vec3>()(vertex.color) << 1U)) >>
            1U) ^
           (std::hash<glm::vec2>()(vertex.texCoord) << 1U);
  }
};
