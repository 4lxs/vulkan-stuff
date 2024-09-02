#include "helpers.hpp"

#include <fmt/format.h>

void vk_check(VkResult err) {
  if (err != VK_SUCCESS) {
    fmt::print("Vulkan error: {}\n", string_VkResult(err));
    exit(1);
  }
}

void sdl_check(bool p_success) {
  if (!p_success) {
    fmt::print("SDL error: {}\n", SDL_GetError());
    exit(1);
  }
}
