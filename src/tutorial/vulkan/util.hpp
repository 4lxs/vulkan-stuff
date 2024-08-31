#pragma once

#include <vk_mem_alloc.h>

#include <vulkan/vulkan.hpp>

#include "../struct.hpp"

namespace vkutil {

void transition_image(VkCommandBuffer cmd, VkImage image,
                      VkImageLayout currentLayout, VkImageLayout newLayout);

void copy_image_to_image(VkCommandBuffer cmd, VkImage source,
                         VkImage destination, VkExtent2D srcSize,
                         VkExtent2D dstSize);

void copy_buffer_to_image(VkCommandBuffer cmd, VkBuffer source,
                          VkImage destination, uint32_t width, uint32_t height);

bool load_shader_module(const char* filePath, VkDevice device,
                        VkShaderModule* outShaderModule);

AllocatedBuffer create_buffer(size_t allocSize, VkBufferUsageFlags usage,
                              VmaMemoryUsage memoryUsage);

}  // namespace vkutil
