#ifndef MYVULKANTRIANGLEAPP_H
#define MYVULKANTRIANGLEAPP_H

#include <SDL3/SDL.h>
#include <vulkan/vulkan_raii.hpp>

class HelloTriangleApplication {
public:
    void run();

private:
    void initWindow();
    void initVulkan();
    void mainLoop();
    void cleanup();
    void createVulkanInstance();

    vk::raii::Context mVulkanContext {};
    vk::raii::Instance mVulkanInstance { nullptr };
    SDL_Window* mpSDLWindow { nullptr };
};

#endif
