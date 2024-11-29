#include "struct.hpp"

#include <fmt/format.h>

#include "engine.hpp"
#include "helpers.hpp"
#include "vulkan/ini.hpp"

AllocatedImage::create(vk::Format format, vk::Extent3D extent,
                       vk::ImageUsageFlags usages,
                       VmaAllocationCreateInfo allocInfo,
                       vk::ImageAspectFlags imageAspect) {
  VkImageCreateInfo rimg_info = vkini::image_create_info(
      VkFormat(format), VkImageUsageFlags(usages), VkExtent3D(extent));

  // for the draw image, we want to allocate it from gpu local memory
  VmaAllocationCreateInfo rimg_allocinfo = {};
  rimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
  rimg_allocinfo.requiredFlags =
      VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  // allocate and create the image
  vmaCreateImage(_allocator, &rimg_info, &rimg_allocinfo, &_drawImage.image,
                 &_drawImage.allocation, nullptr);

  // build a image-view for the draw image to use for rendering
  VkImageViewCreateInfo rview_info = vkini::imageview_create_info(
      _drawImage.format, _drawImage.image, VK_IMAGE_ASPECT_COLOR_BIT);

  vk_check(vkCreateImageView(_device, &rview_info, nullptr, &_drawImage.view));

  return AllocatedImage{
      .extent = extent,
      .format = format,
      .view =
          , .image =, .allocation =,
  };
}

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
