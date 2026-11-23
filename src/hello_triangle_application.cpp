#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan_raii.hpp>

#include "hello_triangle_application.hpp"

void HelloTriangleApplication::run() {
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}

void HelloTriangleApplication::initVulkan() {
    assert(mVulkanInstance == nullptr && "This application has already been initialized");

    constexpr vk::ApplicationInfo appInfo {
        "Hello Triangle", // application name
        VK_MAKE_VERSION(1, 0, 0), // application version
        "No Engine", // engine name
        VK_MAKE_VERSION(1, 0, 0), // engine version
        vk::ApiVersion14, // api version
    };

    vk::InstanceCreateInfo createInfo {
        vk::Flags<vk::InstanceCreateFlagBits> { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO },
        &appInfo
    };
}

void HelloTriangleApplication::initWindow() {
    const bool init { SDL_Init(SDL_INIT_VIDEO) };
    assert(init && "Could not initialize SDL3 library");
    const bool loadVulkanLibrary { SDL_Vulkan_LoadLibrary(nullptr) };
    assert(loadVulkanLibrary && "Could not load Vulkan library");
    mpSDLWindow = SDL_CreateWindow(
        "Vulkan Triangle",
        800, 600,
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY
    );
}

void HelloTriangleApplication::mainLoop() {}

void HelloTriangleApplication::cleanup() {
    SDL_Vulkan_UnloadLibrary();
    SDL_DestroyWindow(mpSDLWindow);
    SDL_Quit();
}

