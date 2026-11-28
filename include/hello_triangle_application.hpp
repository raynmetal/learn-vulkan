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
     * @brief Select the surface format that best suits our needs.
     *
     * Here, we want 32 bit BGRA in standard, non-linear space
     *
     */
    static vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    /**
     * @brief Select the presentation mode best suited for the application.
     *
     * - *eImmediate:* present as soon as result is available, resulting in screen tearing.
     * - *eFifo:* to avoid screen tearing, where presentation happens strictly at "vertical
     *   blanks," similar to vSync; better for mobile devices.
     * - *eFifoRelaxed:* present as soon as possible if last vertical blank had no images
     *   queued.
     * - *eMailbox:* present at blanks, but allow oldest images on queue to be replaced if
     *   newer results are available, known as "triple buffering."
     *
     *
     */
    static vk::PresentModeKHR ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);

    /**
     * @brief Select the resolution of the swap chain images, usually the resolution of the
     * allocated window in pixels.
     *
     * @warning Requires SDL to have allocated a window on our behalf, or this will fail
     *
     */
    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);

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
     * @brief Creates a Vulkan render target whose resources are actually managed by the
     * underlying operating system, where the target is the surface of our application's
     * window.
     *
     */
    void createVulkanSurface();

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
     * @brief Creates an array of images/render targets, of which at any given time at
     * most one is being presented to the screen.
     *
     * The manner of acquiring images to render to, and the order of their presentation,
     * depends on the configuration of our swap chain.
     *
     */
    void createSwapChain();

    /**
     * @brief Retrieves and stores about the best GPU hardware available on this
     * platform.
     *
     */
    void pickPhysicalDevice();

    /**
     * @brief Creates a logical device exposing the capabilities of the selected
     * physical GPU device.
     *
     */
    void createLogicalDevice();

    /**
     * @brief A pointer to some kind of state struct, with wrappers over C-like Vulkan
     * functions describing the current platform.
     *
     */
    vk::raii::Context mVulkanContext {};

    /**
     * @brief Vulkan instance, analogous to OpenGL state context, or the stuff actually
     * keeping track of this application's GPU resources and tags.
     *
     */
    vk::raii::Instance mVulkanInstance { nullptr };

    /**
     * @brief The "surface" of the window allocated to our application by the platform's
     * operating system, a render target for our Vulkan rendering pipeline.
     *
     */
    vk::raii::SurfaceKHR mVulkanSurface { nullptr };

    /**
     * @brief Information about (w.r.t. this application) best GPU hardware available on
     * this platform, to be utilized for rendering.
     *
     */
    vk::raii::PhysicalDevice mVulkanDevicePhysical { nullptr };

    /**
     * @brief Software representation of live GPU being utilized for rendering, with
     * required features explicitly enabled.
     *
     * This is the interface through which GPU resources are allocated and tagged, e.g.,
     * shaders, textures, geometry, etc.
     *
     */
    vk::raii::Device mVulkanDeviceLogical { nullptr };

    /**
     * @brief Command queue on which all graphics related rendering commands will be
     * sent, such as (smaller) buffer updates, depth test versions, fragment or vertex
     * parameter settings, etc.
     *
     */
    vk::raii::Queue mVulkanGraphicsQueue { nullptr };

    /**
     * @brief SDL wrapper over OS specific Window, allowing us to query information about
     * it, receive events related to it, or to draw on it.
     *
     */
    SDL_Window* mpSDLWindow { nullptr };
};

#endif
