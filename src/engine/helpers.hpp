#pragma once

#include <SDL_error.h>

#include <source_location>
#include <vulkan/vulkan.hpp>

void vk_check(vk::Result err, std::source_location loc =
                                  std::source_location::current()) noexcept;

void sdl_check(bool p_success, std::source_location loc =
                                   std::source_location::current()) noexcept;
