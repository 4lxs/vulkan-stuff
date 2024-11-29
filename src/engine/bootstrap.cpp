#include "bootstrap.hpp"

#include <glm/gtx/transform.hpp>

#include "vk_mem_alloc.h"

vkb::Device select_device(vkb::Instance& vkb_inst, vk::SurfaceKHR& surface) {
  VkPhysicalDeviceVulkan13Features features13{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
      .synchronization2 = vk::True,
      .dynamicRendering = vk::True,
  };

  VkPhysicalDeviceVulkan12Features features12{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
  features12.bufferDeviceAddress = vk::True;
  features12.descriptorIndexing = vk::True;

  VkPhysicalDeviceFeatures features{};
  features.samplerAnisotropy = vk::True;

  vkb::PhysicalDeviceSelector selector{vkb_inst};
  vkb::PhysicalDevice physicalDevice = selector.set_minimum_version(1, 3)
                                           .set_required_features_13(features13)
                                           .set_required_features_12(features12)
                                           .set_required_features(features)
                                           .set_surface(surface)
                                           .select()
                                           .value();

  vkb::DeviceBuilder deviceBuilder{physicalDevice};

  return deviceBuilder.build().value();
}
