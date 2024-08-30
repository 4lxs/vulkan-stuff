#include "helpers.hpp"

#include <fmt/format.h>

void vk_check(VkResult err) {
  if (err != VK_SUCCESS) {
    fmt::println("Vulkan error: {}", string_VkResult(err));
    exit(1);
  }
}

void sdl_check(bool p_success) {
  if (!p_success) {
    fmt::println("SDL error: {}", SDL_GetError());
    exit(1);
  }
}
