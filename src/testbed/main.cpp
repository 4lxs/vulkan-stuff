// clang-format off vulkan needs to be included before SDL_vulkan.h
#include <vulkan/vulkan.hpp>
// clang-format on

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <VkBootstrap.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_vulkan.h>
#include <fmt/format.h>
#include <stb/stb_image.h>
#include <vk_mem_alloc.h>

#include <fstream>
#include <glm/gtx/transform.hpp>
#include <string_view>

constexpr uint32_t WINDOW_WIDTH = 1700;
constexpr uint32_t WINDOW_HEIGHT = 900;

void check(vk::Result res) { assert(res == vk::Result::eSuccess); }
vk::ShaderModule load_shader_module(std::string_view filePath,
                                    vk::Device device);

int main() {
  assert(SDL_Init(SDL_INIT_VIDEO) >= 0);

  vk::Extent2D extent{WINDOW_WIDTH, WINDOW_HEIGHT};

  SDL_Window* window = [&]() {
    auto window_flags =
        SDL_WindowFlags(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

    return SDL_CreateWindow("Vulkan Engine", SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED, int(extent.width),
                            int(extent.height), window_flags);
  }();
  assert(window != nullptr);

  vkb::Instance vkbInst = vkb::InstanceBuilder()
                              .set_app_name("vkapp")
                              .request_validation_layers()
                              .use_default_debug_messenger()
                              .require_api_version(1, 3, 0)
                              .build()
                              .value();

  vk::Instance instance = vkbInst.instance;

  vk::SurfaceKHR surface = [&]() {
    VkSurfaceKHR vkSurface{};
    assert(SDL_Vulkan_CreateSurface(window, instance, &vkSurface));
    assert(vkSurface != nullptr);
    return vkSurface;
  }();

  auto [gpu, device, graphicsQueue, graphicsQueueFamily] = [&]() {
    VkPhysicalDeviceVulkan13Features features13{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .synchronization2 = vk::True,
        .dynamicRendering = vk::True,
    };

    VkPhysicalDeviceVulkan12Features features12{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
    features12.bufferDeviceAddress = vk::True;
    features12.descriptorIndexing = vk::True;

    VkPhysicalDeviceFeatures features{};
    features.samplerAnisotropy = vk::True;

    vkb::PhysicalDeviceSelector selector{vkbInst};
    vkb::PhysicalDevice physicalDevice =
        selector.set_minimum_version(1, 3)
            .set_required_features_13(features13)
            .set_required_features_12(features12)
            .set_required_features(features)
            .set_surface(surface)
            .select()
            .value();

    vkb::DeviceBuilder deviceBuilder{physicalDevice};

    vkb::Device vkbDevice = deviceBuilder.build().value();
    return std::make_tuple(
        vk::PhysicalDevice(vkbDevice.physical_device),
        vk::Device(vkbDevice.device),
        vk::Queue(vkbDevice.get_queue(vkb::QueueType::graphics).value()),
        vkbDevice.get_queue_index(vkb::QueueType::graphics).value());
  }();

  VmaAllocator allocator{};
  {
    VmaAllocatorCreateInfo allocatorInfo = {
        .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
        .physicalDevice = gpu,
        .device = device,
        .instance = instance,
    };
    vmaCreateAllocator(&allocatorInfo, &allocator);
  }

  auto [scExtent, swapchain, scImages, scViews] = [&]() {
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

    auto images = vkbSwapchain.get_images().value();
    auto views = vkbSwapchain.get_image_views().value();
    return std::make_tuple(
        vkbSwapchain.extent, vkbSwapchain.swapchain,
        std::vector<vk::Image>{images.begin(), images.end()},
        std::vector<vk::ImageView>{views.begin(), views.end()});
  }();

  struct FrameData {
    vk::Semaphore swapchainSemaphore, renderSemaphore;
    vk::Fence renderFence;

    vk::DescriptorSet descriptorSet;

    vk::CommandPool cmdPool;
    vk::CommandBuffer cmd;
  };
  std::array<FrameData, 2> frames;

  vk::CommandPool immCmdPool;
  vk::CommandBuffer immCmd;

  {
    vk::CommandPoolCreateInfo cpInfo{
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = graphicsQueueFamily,
    };

    for (FrameData& frame : frames) {
      check(device.createCommandPool(&cpInfo, nullptr, &frame.cmdPool));

      vk::CommandBufferAllocateInfo allocInfo{
          .commandPool = frame.cmdPool,
          .level = vk::CommandBufferLevel::ePrimary,
          .commandBufferCount = 1,
      };
      check(device.allocateCommandBuffers(&allocInfo, &frame.cmd));
    }

    check(device.createCommandPool(&cpInfo, nullptr, &immCmdPool));

    vk::CommandBufferAllocateInfo allocInfo{
        .commandPool = immCmdPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1,
    };

    check(device.allocateCommandBuffers(&allocInfo, &immCmd));
  }

  vk::Fence immFence;

  {
    vk::FenceCreateInfo fenceInfo{
        .flags = vk::FenceCreateFlagBits::eSignaled,
    };

    vk::SemaphoreCreateInfo semInfo{};

    check(device.createFence(&fenceInfo, nullptr, &immFence));

    for (auto& frame : frames) {
      check(device.createFence(&fenceInfo, nullptr, &frame.renderFence));

      check(device.createSemaphore(&semInfo, nullptr, &frame.renderSemaphore));

      check(
          device.createSemaphore(&semInfo, nullptr, &frame.swapchainSemaphore));
    }
  }

  vk::ShaderModule fragShader =
      load_shader_module("shaders/triangle.frag.spv", device);
  vk::ShaderModule vertShader =
      load_shader_module("shaders/triangle.vert.spv", device);

  struct Vertex {
    glm::vec3 pos;
    glm::vec4 col;
  };

  {
    vk::VertexInputBindingDescription bindingDesc{
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = vk::VertexInputRate::eVertex,
    };

    std::array attrDesc{
        vk::VertexInputAttributeDescription{
            .location = 0,
            .binding = 0,
            .format = vk::Format::eR32G32B32Sfloat,
            .offset = offsetof(Vertex, pos),
        },
        vk::VertexInputAttributeDescription{
            .location = 1,
            .binding = 0,
            .format = vk::Format::eR32G32B32A32Sfloat,
            .offset = offsetof(Vertex, col),
        },
    };

    vk::PipelineLayoutCreateInfo layoutInfo{};

    vk::GraphicsPipelineCreateInfo pipelineInfo{
        .layout = &layoutInfo,
    };
  }

  return 0;
}

vk::ShaderModule load_shader_module(std::string_view filePath,
                                    vk::Device device) {
  std::ifstream file(filePath.data(), std::ios::ate | std::ios::binary);
  assert(file.is_open());

  ssize_t fileSize = file.tellg();

  // spirv expects the buffer to be on uint32, so make sure to reserve a int
  // vector big enough for the entire file
  std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

  file.seekg(0);

  file.read(reinterpret_cast<char*>(buffer.data()), fileSize);

  file.close();

  vk::ShaderModuleCreateInfo info = {
      .codeSize = buffer.size() * sizeof(uint32_t),
      .pCode = buffer.data(),
  };

  vk::ShaderModule shaderModule;
  check(device.createShaderModule(&info, nullptr, &shaderModule));
  return shaderModule;
}
