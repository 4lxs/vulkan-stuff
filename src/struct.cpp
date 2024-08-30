#include "struct.hpp"

#include <fmt/format.h>

#include "engine.hpp"
#include "helpers.hpp"

AllocatedBuffer::AllocatedBuffer(size_t allocSize, VkBufferUsageFlags usage,
                                 VmaMemoryUsage memoryUsage) {
  VkBufferCreateInfo bufferInfo = {};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.pNext = nullptr;
  bufferInfo.size = allocSize;

  bufferInfo.usage = usage;

  VmaAllocationCreateInfo vmaallocInfo = {};
  vmaallocInfo.usage = memoryUsage;
  vmaallocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

  vk_check(vmaCreateBuffer(Engine::instance()._allocator, &bufferInfo,
                           &vmaallocInfo, &_buffer, &_allocation, &_info));
}

AllocatedBuffer::AllocatedBuffer(AllocatedBuffer&& other) noexcept
    : _buffer{other._buffer},
      _allocation{other._allocation},
      _info{other._info} {
  other._buffer = nullptr;
}

AllocatedBuffer::~AllocatedBuffer() {
  if (_buffer == nullptr) {
    return;
  }
  vmaDestroyBuffer(Engine::instance()._allocator, _buffer, _allocation);
}
