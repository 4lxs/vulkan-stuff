#pragma once

#include <SDL_error.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan_core.h>

void vk_check(VkResult err);

void sdl_check(bool p_success);
