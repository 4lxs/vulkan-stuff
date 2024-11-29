#include "swapchain.hpp"

#include "../engine.hpp"

Swapchain::Swapchain(Engine& engine)
    : Swapchain(engine._windowExtent, engine._gpu, engine._device,
                engine._surface) {}

Swapchain::Swapchain(vk::Extent2D extent, vk::PhysicalDevice& gpu,
                     vk::Device& device, vk::SurfaceKHR& surface) {
  vkb::SwapchainBuilder swapchainBuilder{gpu, device, surface};
  vkb::Swapchain vkbSwapchain =
      swapchainBuilder
          //.use_default_format_selection()
          .set_desired_format(VkSurfaceFormatKHR{
              .format = VK_FORMAT_B8G8R8A8_UNORM,
              .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR})
          // use vsync present mode
          .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
          .set_desired_extent(extent.width, extent.height)
          .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
          .build()
          .value();

  _extent = vkbSwapchain.extent;
  // store swapchain and its related images
  _swapchain = vkbSwapchain.swapchain;
  auto images = vkbSwapchain.get_images().value();
  _images = {images.begin(), images.end()};
  auto views = vkbSwapchain.get_image_views().value();
  _views = {views.begin(), views.end()};
}
