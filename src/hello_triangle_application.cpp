#include <iostream>
#include <string>

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
    createVulkanInstance();
    pickPhysicalDevice();
}

void HelloTriangleApplication::createVulkanInstance() {
    constexpr vk::ApplicationInfo appInfo {
        "Hello Triangle", // application name
        VK_MAKE_VERSION(1, 0, 0), // application version
        "No Engine", // engine name
        VK_MAKE_VERSION(1, 0, 0), // engine version
        vk::ApiVersion14, // api version
    };

    // ====== SET UP VULKAN EXTENSIONS FOR SDL WINDOW RENDERING
    //
    // check which extensions SDL requires to interface with the window system
    uint32_t sdlExtensionCount { 0 };
    auto sdlExtensions { SDL_Vulkan_GetInstanceExtensions(&sdlExtensionCount) };
    // check that all extensions needed by SDL are supported
    auto supportedExtensions { mVulkanContext.enumerateInstanceExtensionProperties() };
    std::cout << "Required SDL Vulkan extensions:";
    for(uint32_t i { 0 }; i < sdlExtensionCount; ++i) {
        std::cout << "\t" << sdlExtensions[i];
        auto supportedIter { std::find_if(
            supportedExtensions.cbegin(), supportedExtensions.cend(),
            [=](const auto& candidate) {
                return std::string{ candidate.extensionName } == std::string{ sdlExtensions[i] };
            }
        ) };
        if(supportedIter == supportedExtensions.cend()) {
            throw std::runtime_error("\nRequired extension not supported: " + std::string{ sdlExtensions[i] });
        }
    }
    std::cout << "\n";

    // ====== SET UP DESIRED VALIDATION LAYERS (BASED ON BUILD CONFIG)
    // 
    //
#ifdef NDEBUG
    const std::vector<const char*> requiredValidation {};
#else
    const std::vector<const char*> requiredValidation {
        "VK_LAYER_KHRONOS_validation",
    };
#endif
    auto supportedLayers { mVulkanContext.enumerateInstanceLayerProperties() };
    std::cout << "Required Vulkan validation layers:";
    for(const auto& layer: requiredValidation) {
        std::cout << "\t" << layer;
        auto supportedIter { std::find_if(
            supportedLayers.cbegin(), supportedLayers.cend(),
            [=](const auto& candidate) {
                return std::string { candidate.layerName } == std::string { layer };
            }
        ) };
        if(supportedIter == supportedLayers.cend()) {
            throw std::runtime_error("\nRequired validation layer not supported: " + std::string{ layer });
        }
    }
    std::cout << "\n";

    vk::InstanceCreateInfo createInfo {
        {},
        &appInfo,
        static_cast<uint32_t>(requiredValidation.size()),
        requiredValidation.data(),
        sdlExtensionCount,
        sdlExtensions
    };

    mVulkanInstance = vk::raii::Instance(mVulkanContext, createInfo);
}

void HelloTriangleApplication::pickPhysicalDevice() {
}

void HelloTriangleApplication::initWindow() {
    const bool init { SDL_Init(SDL_INIT_VIDEO) };
    assert(init && "Could not initialize SDL3 library");
    mpSDLWindow = SDL_CreateWindow(
        "Vulkan Triangle",
        800, 600,
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY
    );
}

void HelloTriangleApplication::mainLoop() {}

void HelloTriangleApplication::cleanup() {
    SDL_DestroyWindow(mpSDLWindow);
    SDL_Quit();
}

