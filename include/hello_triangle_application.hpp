#ifndef MYVULKANTRIANGLEAPP_H
#define MYVULKANTRIANGLEAPP_H

#include <vulkan/vulkan_raii.hpp>
#include <iostream>
#include <stdexcept>
#include <cstdlib>

class HelloTriangleApplication {
public:
    void run();

private:
    void initVulkan();
    void mainLoop();
    void cleanup();
};

#endif
