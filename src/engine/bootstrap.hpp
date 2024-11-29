#pragma once

#include <vulkan/vulkan.hpp>

#include "VkBootstrap.h"
#include "backends/imgui_impl_sdl2.h"

class Engine;

vkb::Device select_device(vkb::Instance& vkb_inst, vk::SurfaceKHR& surface);
