#pragma once

#include <VkBootstrap.h>

#include <vector>
#include <vulkan/vulkan.hpp>

#include "../common.hpp"

class Swapchain {
 public:
  explicit Swapchain(Engine& engine = get_engine());

  Swapchain(vk::Extent2D extent, vk::PhysicalDevice& gpu, vk::Device& device,
            vk::SurfaceKHR& surface);

  Swapchain() = delete;
  Swapchain(const Swapchain&) = delete;
  Swapchain(Swapchain&&) noexcept = default;
  Swapchain& operator=(const Swapchain&) = delete;
  Swapchain& operator=(Swapchain&&) noexcept = delete;
  ~Swapchain() = default;

 private:
  vk::Extent2D _extent;
  vk::SwapchainKHR _swapchain;
  std::vector<vk::Image> _images;
  std::vector<vk::ImageView> _views;
};
