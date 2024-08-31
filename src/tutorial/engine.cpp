#include "engine.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <VkBootstrap.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_vulkan.h>
#include <fmt/format.h>
#include <stb/stb_image.h>
#include <vulkan/vulkan_core.h>

#include <chrono>
#include <glm/gtx/transform.hpp>
#include <iostream>
#include <ranges>
#include <thread>

#include "backends/imgui_impl_sdl2.h"
#include "helpers.hpp"
#include "vk_mem_alloc.h"
#include "vulkan/ini.hpp"
#include "vulkan/util.hpp"

constexpr bool bUseValidationLayers = false;

using namespace std;

static Engine *loadedEngine = nullptr;

Engine::Engine() { init(); }
Engine::~Engine() { cleanup(); }

Engine &Engine::instance() {
  assert(loadedEngine != nullptr);
  return *loadedEngine;
}

void Engine::init() {
  assert(loadedEngine == nullptr);
  loadedEngine = this;

  sdl_check(SDL_Init(SDL_INIT_VIDEO) >= 0);

  auto window_flags = SDL_WindowFlags(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

  _window = SDL_CreateWindow("Vulkan Engine", SDL_WINDOWPOS_UNDEFINED,
                             SDL_WINDOWPOS_UNDEFINED, int(_windowExtent.width),
                             int(_windowExtent.height), window_flags);
  sdl_check(_window != nullptr);

  init_vulkan();
  init_swapchain();
  init_commands();
  init_sync_structures();
  init_descriptor_set_layouts();
  init_pipelines();
  init_texture_image();
  init_texture_sampler();
  init_default_data();
  init_descriptor_pools();
  init_descriptor_sets();

  _isInitialized = true;
}

void Engine::cleanup() {
  assert(_isInitialized);

  // make sure the gpu has stopped doing its things
  vkDeviceWaitIdle(_device);

  _mainDeletionQueue.flush();

  destroy_swapchain();

  vkDestroySurfaceKHR(_instance, _surface, nullptr);
  vkDestroyDevice(_device, nullptr);

  vkb::destroy_debug_utils_messenger(_instance, _debug_messenger);
  vkDestroyInstance(_instance, nullptr);
  SDL_DestroyWindow(_window);
}

void Engine::run() {
  SDL_Event event;
  bool bQuit = false;

  // main loop
  while (!bQuit) {
    // Handle events on queue
    while (SDL_PollEvent(&event) != 0) {
      // close the window when user alt-f4s or clicks the X button
      if (event.type == SDL_QUIT) {
        bQuit = true;
      }

      if (event.type == SDL_WINDOWEVENT) {
        if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
          _resize_requested = true;
        }
        if (event.window.event == SDL_WINDOWEVENT_MINIMIZED) {
          _freeze_rendering = true;
        }
        if (event.window.event == SDL_WINDOWEVENT_RESTORED) {
          _freeze_rendering = false;
        }
      }

      // mainCamera.processSDLEvent(e);
      // ImGui_ImplSDL2_ProcessEvent(&e);
    }

    if (_freeze_rendering) {
      continue;
    }

    // if (_resize_requested) {
    //   resize_swapchain();
    // }

    draw();
  }
}

GPUMeshBuffers Engine::upload_mesh(std::span<const Vertex> vertices,
                                   std::span<const uint16_t> indices) {
  const size_t vertexBufferSize = vertices.size() * sizeof(vertices[0]);
  VkBufferUsageFlags vbUsages{};
  vbUsages |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  vbUsages |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  vbUsages |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
  AllocatedBuffer vertexBuffer(vertexBufferSize, vbUsages,
                               VMA_MEMORY_USAGE_GPU_ONLY);

  const size_t indexBufferSize = indices.size() * sizeof(indices[0]);
  VkBufferUsageFlags ibUsages{};
  ibUsages |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  ibUsages |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
  AllocatedBuffer indexBuffer(indexBufferSize, ibUsages,
                              VMA_MEMORY_USAGE_GPU_ONLY);

  AllocatedBuffer staging{vertexBufferSize + indexBufferSize,
                          VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                          VMA_MEMORY_USAGE_CPU_ONLY};

  vmaCopyMemoryToAllocation(_allocator, vertices.data(), staging._allocation, 0,
                            vertexBufferSize);
  vmaCopyMemoryToAllocation(_allocator, indices.data(), staging._allocation,
                            vertexBufferSize, indexBufferSize);

  immediate_submit([&](VkCommandBuffer cmd) {
    VkBufferCopy vertexCopy{0};
    vertexCopy.dstOffset = 0;
    vertexCopy.srcOffset = 0;
    vertexCopy.size = vertexBufferSize;

    vkCmdCopyBuffer(cmd, staging._buffer, vertexBuffer._buffer, 1, &vertexCopy);

    VkBufferCopy indexCopy{0};
    indexCopy.dstOffset = 0;
    indexCopy.srcOffset = vertexBufferSize;
    indexCopy.size = indexBufferSize;

    vkCmdCopyBuffer(cmd, staging._buffer, indexBuffer._buffer, 1, &indexCopy);
  });

  VkBufferDeviceAddressInfo deviceAdressInfo{
      .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
      .buffer = vertexBuffer._buffer};

  return {std::move(indexBuffer), std::move(vertexBuffer),
          vkGetBufferDeviceAddress(_device, &deviceAdressInfo)};
}

FrameData &Engine::get_current_frame() {
  return _frames.at(_frameNumber % FRAME_OVERLAP);
}

void Engine::init_vulkan() {
  VkDebugUtilsMessageSeverityFlagsEXT severity{};
  severity |= VK_DEBUG_REPORT_DEBUG_BIT_EXT;
  severity |= VK_DEBUG_REPORT_WARNING_BIT_EXT;
  severity |= VK_DEBUG_REPORT_ERROR_BIT_EXT;
  // make the vulkan instance, with basic debug features
  auto inst_ret = vkb::InstanceBuilder()
                      .set_app_name("Example Vulkan Application")
                      .request_validation_layers(bUseValidationLayers)
                      .set_debug_messenger_severity(severity)
                      .use_default_debug_messenger()
                      .require_api_version(1, 3, 0)
                      .build();

  vkb::Instance vkb_inst = inst_ret.value();

  // grab the instance
  _instance = vkb_inst.instance;
  _debug_messenger = vkb_inst.debug_messenger;

  sdl_check(SDL_Vulkan_CreateSurface(_window, _instance, &_surface) ==
            SDL_TRUE);
  assert(_surface != nullptr);

  VkPhysicalDeviceVulkan13Features features{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
  features.dynamicRendering = true;
  features.synchronization2 = true;

  VkPhysicalDeviceVulkan12Features features12{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
  features12.bufferDeviceAddress = true;
  features12.descriptorIndexing = true;

  vkb::PhysicalDeviceSelector selector{vkb_inst};
  vkb::PhysicalDevice physicalDevice = selector.set_minimum_version(1, 3)
                                           .set_required_features_13(features)
                                           .set_required_features_12(features12)
                                           .set_surface(_surface)
                                           .select()
                                           .value();

  vkb::DeviceBuilder deviceBuilder{physicalDevice};
  vkb::Device vkbDevice = deviceBuilder.build().value();

  _device = vkbDevice.device;
  _gpu = physicalDevice.physical_device;
  _graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
  _graphicsQueueFamily =
      vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

  VmaAllocatorCreateInfo allocatorInfo = {};
  allocatorInfo.physicalDevice = _gpu;
  allocatorInfo.device = _device;
  allocatorInfo.instance = _instance;
  allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
  vmaCreateAllocator(&allocatorInfo, &_allocator);

  _mainDeletionQueue.push_function([&]() { vmaDestroyAllocator(_allocator); });
}

void Engine::init_swapchain() {
  create_swapchain(_windowExtent.width, _windowExtent.height);

  VkExtent3D drawImageExtent = {_windowExtent.width, _windowExtent.height, 1};

  _drawImage.format = VK_FORMAT_R16G16B16A16_SFLOAT;
  _drawImage.extent = drawImageExtent;

  VkImageUsageFlags drawImageUsages{};
  drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
  drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  VkImageCreateInfo rimg_info = vkini::image_create_info(
      _drawImage.format, drawImageUsages, drawImageExtent);

  // for the draw image, we want to allocate it from gpu local memory
  VmaAllocationCreateInfo rimg_allocinfo = {};
  rimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
  rimg_allocinfo.requiredFlags =
      VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  // allocate and create the image
  vmaCreateImage(_allocator, &rimg_info, &rimg_allocinfo, &_drawImage.image,
                 &_drawImage.allocation, nullptr);

  // build a image-view for the draw image to use for rendering
  VkImageViewCreateInfo rview_info = vkini::imageview_create_info(
      _drawImage.format, _drawImage.image, VK_IMAGE_ASPECT_COLOR_BIT);

  vk_check(vkCreateImageView(_device, &rview_info, nullptr, &_drawImage.view));

  _mainDeletionQueue.push_function([this]() {
    vkDestroyImageView(_device, _drawImage.view, nullptr);
    vmaDestroyImage(_allocator, _drawImage.image, _drawImage.allocation);
  });
}

void Engine::init_commands() {
  // create a command pool for commands submitted to the graphics queue.
  // we also want the pool to allow for resetting of individual command buffers
  VkCommandPoolCreateInfo commandPoolInfo = vkini::command_pool_create_info(
      _graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

  for (FrameData &frame : _frames) {
    vk_check(vkCreateCommandPool(_device, &commandPoolInfo, nullptr,
                                 &frame._commandPool));

    _mainDeletionQueue.push_function([=, this]() {
      vkDestroyCommandPool(_device, frame._commandPool, nullptr);
    });

    // allocate the default command buffer that we will use for rendering
    VkCommandBufferAllocateInfo cmdAllocInfo =
        vkini::command_buffer_allocate_info(frame._commandPool, 1);

    vk_check(vkAllocateCommandBuffers(_device, &cmdAllocInfo,
                                      &frame._mainCommandBuffer));
  }

  vk_check(vkCreateCommandPool(_device, &commandPoolInfo, nullptr,
                               &_immCommandPool));

  // allocate the default command buffer that we will use for rendering
  VkCommandBufferAllocateInfo cmdAllocInfo =
      vkini::command_buffer_allocate_info(_immCommandPool, 1);

  vk_check(
      vkAllocateCommandBuffers(_device, &cmdAllocInfo, &_immCommandBuffer));

  _mainDeletionQueue.push_function(
      [this]() { vkDestroyCommandPool(_device, _immCommandPool, nullptr); });
}

void Engine::init_sync_structures() {
  VkFenceCreateInfo fenceCreateInfo =
      vkini::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);
  vk_check(vkCreateFence(_device, &fenceCreateInfo, nullptr, &_immFence));
  VkSemaphoreCreateInfo semaphoreCreateInfo = vkini::semaphore_create_info();

  _mainDeletionQueue.push_function(
      [this]() { vkDestroyFence(_device, _immFence, nullptr); });

  for (FrameData &frame : _frames) {
    vk_check(
        vkCreateFence(_device, &fenceCreateInfo, nullptr, &frame._renderFence));

    vk_check(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr,
                               &frame._swapchainSemaphore));
    vk_check(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr,
                               &frame._renderSemaphore));

    _mainDeletionQueue.push_function([=, this]() {
      vkDestroyFence(_device, frame._renderFence, nullptr);
      vkDestroySemaphore(_device, frame._swapchainSemaphore, nullptr);
      vkDestroySemaphore(_device, frame._renderSemaphore, nullptr);
    });
  }
}

void Engine::init_descriptor_set_layouts() {
  VkDescriptorSetLayoutBinding uboLayoutBinding{};
  uboLayoutBinding.binding = 0;
  uboLayoutBinding.descriptorCount = 1;
  uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  uboLayoutBinding.pImmutableSamplers = nullptr;
  uboLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  VkDescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = 1;
  layoutInfo.pBindings = &uboLayoutBinding;

  vk_check(vkCreateDescriptorSetLayout(_device, &layoutInfo, nullptr,
                                       &_descriptorSetLayout));

  _mainDeletionQueue.push_function([this]() {
    vkDestroyDescriptorSetLayout(_device, _descriptorSetLayout, nullptr);
  });
}

void Engine::init_pipelines() {
  _triangle.emplace();
  _triangle->build_pipeline();

  _mainDeletionQueue.push_function([&]() { _triangle = std::nullopt; });
}

void Engine::init_texture_image() {
  int texWidth{};
  int texHeight{};
  int texChannels{};

  stbi_uc *pixels = stbi_load("textures/texture.jpg", &texWidth, &texHeight,
                              &texChannels, STBI_rgb_alpha);

  if (pixels == nullptr) {
    throw std::runtime_error("failed to load texture image!");
  }

  size_t imageSize = static_cast<size_t>(texWidth) * texHeight * 4;

  _textureImage.format = VK_FORMAT_R8G8B8A8_SRGB;
  _textureImage.extent = VkExtent3D{static_cast<uint32_t>(texWidth),
                                    static_cast<uint32_t>(texHeight), 1};

  AllocatedBuffer staging{imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                          VMA_MEMORY_USAGE_CPU_ONLY};

  vmaCopyMemoryToAllocation(_allocator, pixels, staging._allocation, 0,
                            static_cast<size_t>(imageSize));

  stbi_image_free(pixels);

  VkImageCreateInfo img_create_info = vkini::image_create_info(
      _textureImage.format,
      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
      _textureImage.extent);

  VmaAllocationCreateInfo img_alloc_info = {};
  img_alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
  img_alloc_info.requiredFlags =
      VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  vmaCreateImage(_allocator, &img_create_info, &img_alloc_info,
                 &_textureImage.image, &_textureImage.allocation, nullptr);

  immediate_submit([&](VkCommandBuffer cmd) {
    vkutil::transition_image(cmd, _textureImage.image,
                             VK_IMAGE_LAYOUT_UNDEFINED,
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    vkutil::copy_buffer_to_image(cmd, staging._buffer, _textureImage.image,
                                 texWidth, texHeight);

    vkutil::transition_image(cmd, _textureImage.image,
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                             VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  });

  VkImageViewCreateInfo view_info = vkini::imageview_create_info(
      _textureImage.format, _textureImage.image, VK_IMAGE_ASPECT_COLOR_BIT);

  vk_check(
      vkCreateImageView(_device, &view_info, nullptr, &_textureImage.view));

  _mainDeletionQueue.push_function([this]() {
    vkDestroyImageView(_device, _textureImage.view, nullptr);
    vmaDestroyImage(_allocator, _textureImage.image, _textureImage.allocation);
  });
}

void Engine::init_texture_sampler() {
  VkPhysicalDeviceProperties properties{};
  vkGetPhysicalDeviceProperties(_gpu, &properties);

  VkSamplerCreateInfo samplerInfo{};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.magFilter = VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_LINEAR;
  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.anisotropyEnable = VK_TRUE;
  samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
  samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  samplerInfo.unnormalizedCoordinates = VK_FALSE;
  samplerInfo.compareEnable = VK_FALSE;
  samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerInfo.minLod = 0.;
  samplerInfo.maxLod = VK_LOD_CLAMP_NONE;
  samplerInfo.mipLodBias = 0.;

  vk_check(vkCreateSampler(_device, &samplerInfo, nullptr, &_textureSampler));

  _mainDeletionQueue.push_function(
      [this]() { vkDestroySampler(_device, _textureSampler, nullptr); });
}

void Engine::init_default_data() { _triangle->init_data(); }

void Engine::init_descriptor_pools() {
  std::array poolSizes{VkDescriptorPoolSize{
      .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .descriptorCount = static_cast<uint32_t>(FRAME_OVERLAP)}};

  VkDescriptorPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  poolInfo.pPoolSizes = poolSizes.data();
  poolInfo.maxSets = static_cast<uint32_t>(FRAME_OVERLAP);

  vk_check(
      vkCreateDescriptorPool(_device, &poolInfo, nullptr, &_descriptorPool));

  _mainDeletionQueue.push_function(
      [this]() { vkDestroyDescriptorPool(_device, _descriptorPool, nullptr); });
}

void Engine::init_descriptor_sets() {
  std::vector<VkDescriptorSetLayout> layouts(FRAME_OVERLAP,
                                             _descriptorSetLayout);
  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = _descriptorPool;
  allocInfo.descriptorSetCount = static_cast<uint32_t>(FRAME_OVERLAP);
  allocInfo.pSetLayouts = layouts.data();

  _descriptorSets.resize(FRAME_OVERLAP);
  vk_check(
      vkAllocateDescriptorSets(_device, &allocInfo, _descriptorSets.data()));

  for (int i = 0; i < FRAME_OVERLAP; i++) {
    VkDescriptorSet set = _frames.at(i)._descriptorSet = _descriptorSets.at(i);
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = _textureImage.view;
    imageInfo.sampler = _textureSampler;

    std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = set;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(_device,
                           static_cast<uint32_t>(descriptorWrites.size()),
                           descriptorWrites.data(), 0, nullptr);
  }
}

void Engine::create_swapchain(uint32_t width, uint32_t height) {
  vkb::SwapchainBuilder swapchainBuilder{_gpu, _device, _surface};

  _swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

  vkb::Swapchain vkbSwapchain =
      swapchainBuilder
          //.use_default_format_selection()
          .set_desired_format(VkSurfaceFormatKHR{
              .format = _swapchainImageFormat,
              .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR})
          // use vsync present mode
          .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
          .set_desired_extent(width, height)
          .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
          .build()
          .value();

  _swapchainExtent = vkbSwapchain.extent;
  // store swapchain and its related images
  _swapchain = vkbSwapchain.swapchain;
  _swapchainImages = vkbSwapchain.get_images().value();
  _swapchainImageViews = vkbSwapchain.get_image_views().value();
}

void Engine::destroy_swapchain() {
  vkDestroySwapchainKHR(_device, _swapchain, nullptr);

  for (const VkImageView &imageview : _swapchainImageViews) {
    vkDestroyImageView(_device, imageview, nullptr);
  }
}

void Engine::draw() {
  // wait until the gpu has finished rendering the last frame. Timeout of 1
  // second
  FrameData &frame = get_current_frame();
  vk_check(vkWaitForFences(_device, 1, &frame._renderFence, true, 1000000000));
  vk_check(vkResetFences(_device, 1, &frame._renderFence));

  frame._deletionQueue.flush();

  // request image from the swapchain
  uint32_t swapchainImageIndex{};

  VkResult e = vkAcquireNextImageKHR(_device, _swapchain, 1000000000,
                                     frame._swapchainSemaphore, nullptr,
                                     &swapchainImageIndex);
  if (e == VK_ERROR_OUT_OF_DATE_KHR) {
    _resize_requested = true;
    return;
  }
  vk_check(e);

  vk_check(vkResetCommandBuffer(frame._mainCommandBuffer, 0));

  VkCommandBuffer cmd = frame._mainCommandBuffer;

  VkCommandBufferBeginInfo cmdBeginInfo = vkini::command_buffer_begin_info(
      VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

  _drawExtent.width = _drawImage.extent.width;
  _drawExtent.height = _drawImage.extent.height;

  vk_check(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

  // we will overwrite it all so we dont care about what was the older layout
  vkutil::transition_image(cmd, _drawImage.image, VK_IMAGE_LAYOUT_UNDEFINED,
                           VK_IMAGE_LAYOUT_GENERAL);

  draw_background(cmd);

  vkutil::transition_image(cmd, _drawImage.image, VK_IMAGE_LAYOUT_GENERAL,
                           VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL);

  draw_geometry(cmd);

  vkutil::transition_image(cmd, _drawImage.image,
                           VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  vkutil::transition_image(cmd, _swapchainImages[swapchainImageIndex],
                           VK_IMAGE_LAYOUT_UNDEFINED,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  vkutil::copy_image_to_image(cmd, _drawImage.image,
                              _swapchainImages[swapchainImageIndex],
                              _drawExtent, _swapchainExtent);

  vkutil::transition_image(cmd, _swapchainImages[swapchainImageIndex],
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

  vk_check(vkEndCommandBuffer(cmd));

  VkCommandBufferSubmitInfo cmdinfo = vkini::command_buffer_submit_info(cmd);

  VkSemaphoreSubmitInfo waitInfo = vkini::semaphore_submit_info(
      VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
      frame._swapchainSemaphore);
  VkSemaphoreSubmitInfo signalInfo = vkini::semaphore_submit_info(
      VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, frame._renderSemaphore);

  VkSubmitInfo2 submit = vkini::submit_info(&cmdinfo, &signalInfo, &waitInfo);

  vk_check(vkQueueSubmit2(_graphicsQueue, 1, &submit, frame._renderFence));

  VkPresentInfoKHR presentInfo = vkini::present_info();

  presentInfo.pSwapchains = &_swapchain;
  presentInfo.swapchainCount = 1;

  presentInfo.pWaitSemaphores = &frame._renderSemaphore;
  presentInfo.waitSemaphoreCount = 1;

  presentInfo.pImageIndices = &swapchainImageIndex;

  VkResult presentResult = vkQueuePresentKHR(_graphicsQueue, &presentInfo);
  // increase the number of frames drawn
  _frameNumber++;
}

void Engine::draw_background(VkCommandBuffer cmd) {
  // make a clear-color from frame number. This will flash with a 120 frame
  // period.
  VkClearColorValue clearValue;
  float flash = std::abs(std::sin(_frameNumber / 120.f));
  clearValue = {{0.0f, 0.0f, flash, 1.0f}};

  VkImageSubresourceRange clearRange =
      vkini::image_subresource_range(VK_IMAGE_ASPECT_COLOR_BIT);

  vkCmdClearColorImage(cmd, _drawImage.image, VK_IMAGE_LAYOUT_GENERAL,
                       &clearValue, 1, &clearRange);
}

void Engine::draw_geometry(VkCommandBuffer cmd) {
  // begin a render pass  connected to our draw image
  VkRenderingAttachmentInfo colorAttachment = vkini::attachment_info(
      _drawImage.view, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

  VkRenderingInfo renderInfo =
      vkini::rendering_info(_drawExtent, &colorAttachment, nullptr);
  vkCmdBeginRendering(cmd, &renderInfo);

  // set dynamic viewport and scissor
  VkViewport viewport = {};
  viewport.x = 0;
  viewport.y = 0;
  viewport.width = float(_drawExtent.width);
  viewport.height = float(_drawExtent.height);
  viewport.minDepth = 0.;
  viewport.maxDepth = 1.;

  vkCmdSetViewport(cmd, 0, 1, &viewport);

  VkRect2D scissor = {};
  scissor.offset.x = 0;
  scissor.offset.y = 0;
  scissor.extent.width = _drawExtent.width;
  scissor.extent.height = _drawExtent.height;

  vkCmdSetScissor(cmd, 0, 1, &scissor);

  _triangle->draw(cmd);

  vkCmdEndRendering(cmd);
}

void Engine::immediate_submit(
    std::function<void(VkCommandBuffer cmd)> &&function) {
  vk_check(vkResetFences(_device, 1, &_immFence));
  vk_check(vkResetCommandBuffer(_immCommandBuffer, 0));

  VkCommandBuffer cmd = _immCommandBuffer;
  VkCommandBufferBeginInfo cmdBeginInfo = vkini::command_buffer_begin_info(
      VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

  vk_check(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

  function(cmd);

  vk_check(vkEndCommandBuffer(cmd));

  VkCommandBufferSubmitInfo cmdinfo = vkini::command_buffer_submit_info(cmd);
  VkSubmitInfo2 submit = vkini::submit_info(&cmdinfo, nullptr, nullptr);

  // submit command buffer to the queue and execute it.
  //  _renderFence will now block until the graphic commands finish execution
  vk_check(vkQueueSubmit2(_graphicsQueue, 1, &submit, _immFence));

  vk_check(vkWaitForFences(_device, 1, &_immFence, true, 9999999999));
}
