#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <array>
#include <deque>
#include <functional>
#include <glm/glm.hpp>
#include <optional>
#include <ranges>
#include <span>
#include <vector>

#include "object.hpp"
#include "struct.hpp"

struct DeletionQueue {
  std::deque<std::function<void()>> deletors;

  void push_function(std::function<void()> &&function) {
    deletors.push_back(function);
  }

  void flush() {
    for (auto &deletor : std::ranges::reverse_view(deletors)) {
      deletor();
    }

    deletors.clear();
  }
};

struct FrameData {
  VkSemaphore _swapchainSemaphore, _renderSemaphore;
  VkFence _renderFence;

  DeletionQueue _deletionQueue;
  VkDescriptorSet _descriptorSet;

  VkCommandPool _commandPool;
  VkCommandBuffer _mainCommandBuffer;
};

constexpr unsigned int FRAME_OVERLAP = 2;

class Engine {
 public:
  Engine();
  Engine(const Engine &) = delete;
  Engine(Engine &&) = delete;
  Engine &operator=(const Engine &) = delete;
  Engine &operator=(Engine &&) = delete;
  ~Engine();

  // singleton style getter. multiple engines is not supported
  static Engine &instance();

  // run main loop
  void run();

  [[nodiscard]] GPUMeshBuffers upload_mesh(std::span<const Vertex> vertices,
                                           std::span<const uint16_t> indices);

  FrameData &get_current_frame();

 private:
  // initializes everything in the engine
  void init();

  // shuts down the engine
  void cleanup();

  void init_vulkan();

  void init_swapchain();

  void init_commands();

  void init_sync_structures();

  void init_descriptor_set_layouts();

  void init_pipelines();

  void init_texture_image();

  void init_texture_sampler();

  void init_default_data();

  void init_descriptor_pools();

  void init_descriptor_sets();

  void create_swapchain(uint32_t width, uint32_t height);

  void destroy_swapchain();

  // draw loop
  void draw();

  void draw_background(VkCommandBuffer cmd);

  void draw_geometry(VkCommandBuffer cmd);

  void immediate_submit(std::function<void(VkCommandBuffer cmd)> &&function);

 public:
  bool _isInitialized{false};
  int _frameNumber{0};
  bool _resize_requested{false};
  bool _freeze_rendering{false};

  VkExtent2D _windowExtent{1700, 900};

  struct SDL_Window *_window{};

  VkInstance _instance{};
  VkDebugUtilsMessengerEXT _debug_messenger{};
  VkPhysicalDevice _gpu{};
  VkDevice _device{};

  VkQueue _graphicsQueue{};
  uint32_t _graphicsQueueFamily{};

  std::array<FrameData, FRAME_OVERLAP> _frames{};

  VkSurfaceKHR _surface{};
  VkSwapchainKHR _swapchain{};
  VkFormat _swapchainImageFormat{};
  VkExtent2D _swapchainExtent{};
  VkExtent2D _drawExtent{};
  VkDescriptorPool _descriptorPool{};

  std::vector<VkImage> _swapchainImages;
  std::vector<VkImageView> _swapchainImageViews;

  DeletionQueue _mainDeletionQueue{};

  VmaAllocator _allocator{};

  AllocatedImage _drawImage{};

  VkFence _immFence{};
  VkCommandBuffer _immCommandBuffer{};
  VkCommandPool _immCommandPool{};

  std::optional<TriangleObject> _triangle;

  AllocatedImage _textureImage{};
  VkSampler _textureSampler{};

  VkDescriptorSetLayout _descriptorSetLayout{};
  std::vector<VkDescriptorSet> _descriptorSets;
};
