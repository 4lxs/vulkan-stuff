#include "helpers.hpp"

#include <fmt/format.h>
#include <spdlog/spdlog.h>

void vk_check(vk::Result err, std::source_location loc) noexcept {
  if (err != vk::Result::eSuccess) {
    spdlog::error("Vulkan error: {} {}: {}\n", loc.file_name(), loc.line(),
                  vk::to_string(err));
    std::terminate();
  }
}

void sdl_check(bool p_success, std::source_location loc) noexcept {
  if (!p_success) {
    spdlog::error("SDL error: {} {}: {}\n", loc.file_name(), loc.line(),
                  SDL_GetError());
    std::terminate();
  }
}
