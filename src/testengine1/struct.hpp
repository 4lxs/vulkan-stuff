#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <glm/glm.hpp>

struct AllocatedImage {
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
};
