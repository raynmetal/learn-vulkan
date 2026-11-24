#ifndef MYVULKANTRIANGLEAPP_H
#define MYVULKANTRIANGLEAPP_H

#include <SDL3/SDL.h>
#include <vulkan/vulkan_raii.hpp>

class HelloTriangleApplication {
public:
    /**
     * @brief Function that initializes SDL and this application's Window, and manages
     * the application loop.
     *
     */
    void run();

private:
    /**
     * @brief Makes SDL create a Vulkan-compatible window on our behalf
     *
     */
    void initWindow();

    /**
     * @brief Sets up a Vulkan instance, enabling Vulkan extensions necessary for SDL
     *
     */
    void initVulkan();

    /**
     * @brief Loop scheduling rendering calls and application udpates as fast as possible
     * on the current platform.
     *
     */
    void mainLoop();

    /**
     * @brief Safe destruction of resources allocated through C-based APIs, such as
     * those provided by SDL
     *
     */
    void cleanup();

    /**
     * @brief As part of initVulkan, creates a Vulkan "instance," analogous to OpenGL
     * state context.
     *
     */
    void createVulkanInstance();

    /**
     * @brief Retrieves and stores software representation of best GPU hardware
     * available on this platform.
     *
     */
    void pickPhysicalDevice();

    /**
     * @brief A pointer to some kind of state struct, with wrappers over C-like vulkan
     * functions describing the current platform.
     */
    vk::raii::Context mVulkanContext {};

    /**
     * @brief Vulkan instance, analogous to OpenGL state context, or the stuff actually
     * keeping track of GPU-allocated resources and their CPU handles.
     *
     */
    vk::raii::Instance mVulkanInstance { nullptr };

    /**
     * @brief Software representation of GPU hardware available on this platform being
     * utilized for rendering.
     *
     */
    vk::raii::PhysicalDevice mVulkanDevice { nullptr };

    /**
     * @brief SDL wrapper over OS specific Window, allowing us to query information about
     * it, receive events related to it, or to draw on it.
     *
     */
    SDL_Window* mpSDLWindow { nullptr };
};

#endif
