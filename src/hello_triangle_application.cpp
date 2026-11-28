#include <iostream>
#include <map>
#include <string>
#include <limits>
#include <algorithm>

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
    createVulkanSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
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
    uint32_t sdlExtensionCount;
    const auto sdlExtensions { SDL_Vulkan_GetInstanceExtensions(&sdlExtensionCount) };
    // check that all extensions needed by SDL are supported
    const auto supportedExtensions { mVulkanContext.enumerateInstanceExtensionProperties() };
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
    const auto supportedLayers { mVulkanContext.enumerateInstanceLayerProperties() };
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

void HelloTriangleApplication::createVulkanSurface() {
    // SDL's Vulkan surface creation doesn't understand vk::raii, so we use
    // the C interface here.
    VkSurfaceKHR _surface;
    SDL_Vulkan_CreateSurface(mpSDLWindow, *mVulkanInstance, nullptr, &_surface);

    mVulkanSurface = vk::raii::SurfaceKHR(mVulkanInstance, _surface);
}

void HelloTriangleApplication::createSwapChain() {
    // Get supported swap chain configuration taking into account device capabilities
    // and the Vulkan surface we've been provided.
    const auto surfaceCapabilities {
        mVulkanDevicePhysical.getSurfaceCapabilitiesKHR(*mVulkanSurface)
    };
    const auto availableFormats {
        mVulkanDevicePhysical.getSurfaceFormatsKHR(*mVulkanSurface)
    };
    const auto availablePresentModes {
        mVulkanDevicePhysical.getSurfacePresentModesKHR(*mVulkanSurface)
    };
    const uint32_t minImageCount {
        ChooseSwapMinImageCount(surfaceCapabilities)
    };
    const vk::PresentModeKHR presentMode {
        ChooseSwapPresentMode(availablePresentModes)
    };
    mSwapChainExtent = chooseSwapExtent(surfaceCapabilities);
    mSwapChainSurfaceFormat = ChooseSwapSurfaceFormat(availableFormats);

    // Configure swap chain construction parameter struct
    vk::SwapchainCreateInfoKHR swapChainCreateInfo { };
    swapChainCreateInfo.surface = *mVulkanSurface;
    swapChainCreateInfo.minImageCount = minImageCount;
    swapChainCreateInfo.imageFormat = mSwapChainSurfaceFormat.format;
    swapChainCreateInfo.imageColorSpace = mSwapChainSurfaceFormat.colorSpace;
    swapChainCreateInfo.imageExtent = mSwapChainExtent;
    // number of layers each image consists of, usually 1 unless application
    // uses stereoscopic 3D
    swapChainCreateInfo.imageArrayLayers = 1;
    // we render colors to the images, hence, they're color attachments
    swapChainCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
    // only one queue family at a time may possess and write to an image
    swapChainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
    // transform applied to images in the swap chain, if supported
    swapChainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
    // whether this window will need to be blended with other windows by the window
    // manager (in our case, it won't)
    swapChainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    swapChainCreateInfo.presentMode = presentMode;
    // we don't care about accurately reading colors on pixels that are obscured
    // by other windows, or outside the boundaries of the desktop
    swapChainCreateInfo.clipped = true;
    // in case we're creating a new swap chain in response to a resize event,
    // a pointer to the old swap chain
    swapChainCreateInfo.oldSwapchain = nullptr;

    // actually create and store reference to the swap chain and its images
    mSwapChain = vk::raii::SwapchainKHR{ mVulkanDeviceLogical, swapChainCreateInfo };
    mSwapChainImages = mSwapChain.getImages();
}

void HelloTriangleApplication::pickPhysicalDevice() {
    const std::vector<const char*> requiredDeviceExtensions {
        vk::KHRSwapchainExtensionName, // ability to present images to the screen
    };
    const auto physicalDevices { mVulkanInstance.enumeratePhysicalDevices() };
    if(physicalDevices.empty()) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    // use an ordered map to sort candidates by increasing score, then
    // pick the device with the highest score
    std::multimap<int, vk::raii::PhysicalDevice> candidates {};
    for(const auto& device: physicalDevices) {

        const auto deviceProperties { device.getProperties() };
        const auto deviceFeatures { device.getFeatures() };

        std::cout << "Checking device " << deviceProperties.deviceName << "\n";

        // guard: ensure our Vulkan API version is supported
        const bool supportsVulkan1_3 {
            deviceProperties.apiVersion >= vk::ApiVersion13
        };
        if(!supportsVulkan1_3) {
            std::cout << "\tDoes not support Vulkan 1.3 API\n";
            continue;
        }
        std::cout << "\tSupports Vulkan 1.3 API\n";

        // guard: ensure graphics work queues are supported
        const auto queueFamilies { device.getQueueFamilyProperties() };
        const bool supportsGraphics {
            std::find_if(
                queueFamilies.cbegin(), queueFamilies.cend(),
                [](const auto& queueFamily) {
                    return !!(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics);
                }
            ) != queueFamilies.cend()
        };
        if(!supportsGraphics) {
            std::cout << "\tDoes not support graphics work\n";
            continue;
        }
        std::cout << "\tSupports graphics work\n";

        // guard: ensure required extensions are available
        const auto availableExtensions { device.enumerateDeviceExtensionProperties() };
        const bool supportsRequiredExtensions { [&](){
            for(const auto& extension: requiredDeviceExtensions) {
                const bool isUnsupported {
                    std::find_if(availableExtensions.cbegin(), availableExtensions.cend(),
                        [&](const auto& availableExtension) {
                            return std::string{ availableExtension.extensionName } == extension;
                        }
                    ) == availableExtensions.cend()
                };
                if(isUnsupported) { return false; }
            }
            return true;
        } () };
        if(!supportsRequiredExtensions) {
            std::cout << "\tDoes not support required extensions\n";
            continue;
        }
        std::cout << "\tSupports required extensions\n";

        // checkpoint: basic requirements met; score and add to candidate list

        // compute device score relative to other devices
        uint32_t score { 0 };
        // discrete GPUs are better
        if(deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
            score += 1000;
        }
        // more texture memory is better
        score += deviceProperties.limits.maxImageDimension2D;

        candidates.insert(std::make_pair(score, device));
    }

    if(candidates.empty()) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }

    std::cout << "Device "
        << candidates.rbegin()->second.getProperties().deviceName
        << " chosen.\n";
    mVulkanDevicePhysical = candidates.rbegin()->second;
}

void HelloTriangleApplication::createLogicalDevice() {
    const std::vector<vk::QueueFamilyProperties> queueFamilyProperties {
        mVulkanDevicePhysical.getQueueFamilyProperties()
    };

    // specify that we want to create a graphics command queue
    uint32_t potentialGraphicsQueueIndex { static_cast<uint32_t>(~0) };
    for(auto i { 0 }; i < queueFamilyProperties.size(); ++i) {
        const auto& queue { queueFamilyProperties[i] };
        const bool supportsGraphics {
            !!(queue.queueFlags & vk::QueueFlagBits::eGraphics)
        };
        const bool supportsPresent { static_cast<bool>(
            mVulkanDevicePhysical.getSurfaceSupportKHR(i, *mVulkanSurface)
        ) };
        if(supportsGraphics && supportsPresent) {
            potentialGraphicsQueueIndex = i;
            break;
        }
    }
    if(potentialGraphicsQueueIndex == ~0) {
        throw std::runtime_error(
            "Could not find a queue for graphics and present -> terminating"
        );
    }
    const auto graphicsQueueIndex { potentialGraphicsQueueIndex };
    const std::array<float, 1> queuePriorities {
        1.f,
    };
    const vk::DeviceQueueCreateInfo deviceQueueCreateInfo {
        {}, // flags
        graphicsQueueIndex, // queue index
        1,
        queuePriorities.data()
    };

    // NOTE: In future -- just use default constructor, then modify whichever
    // fields I want *afterwards*.  Specifying which fields I'd like to customize
    // at construction appears to be unreasonable here.
    //
    // specify which vulkan features we'd like to use
    const vk::StructureChain<
        vk::PhysicalDeviceFeatures2,
        vk::PhysicalDeviceVulkan11Features,
        vk::PhysicalDeviceVulkan13Features,
        vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
    > featureChain {
        {},
        vk::PhysicalDeviceVulkan11Features {
            {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {},
            true, {}
        },
        vk::PhysicalDeviceVulkan13Features {
            {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {},
            true, {}, {}, {}
        },
        vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT {
            true
        },
    };

    // specify which hardware extensions that we'd like our device to use
    std::vector<const char*> requiredDeviceExtensions {
        vk::KHRSwapchainExtensionName
    };

    vk::DeviceCreateInfo deviceCreateInfo {
        {},
        1,
        &deviceQueueCreateInfo,
        {},
        {},
        static_cast<uint32_t>(requiredDeviceExtensions.size()),
        requiredDeviceExtensions.data(),
        {},
        featureChain.get<vk::PhysicalDeviceFeatures2>(),
    };

    mVulkanDeviceLogical = vk::raii::Device{ mVulkanDevicePhysical, deviceCreateInfo };

    mVulkanGraphicsQueue = vk::raii::Queue{
        mVulkanDeviceLogical,
        graphicsQueueIndex,
        0, // The index of the only queue we've created for our logical device
    };
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
    // swap chain destructor causes seg fault (I think) because it's called after
    // the destruction of its owning Vulkan instance
    mSwapChainImages.clear();
    mSwapChain = nullptr;

    SDL_DestroyWindow(mpSDLWindow);
    SDL_Quit();
}

vk::SurfaceFormatKHR HelloTriangleApplication::ChooseSwapSurfaceFormat(
    const std::vector<vk::SurfaceFormatKHR>& availableFormats
) {
    assert(!availableFormats.empty());

    constexpr vk::SurfaceFormatKHR desiredFormat {
        vk::Format::eB8G8R8A8Srgb,
        vk::ColorSpaceKHR::eSrgbNonlinear,
    };
    const auto selectedFormat { std::find(
        availableFormats.cbegin(), availableFormats.cend(),
        desiredFormat
    ) };

    // guard: our top choice isn't present, return whatever
    if(selectedFormat == availableFormats.cend()) {
        return availableFormats[0];
    }

    // return our top choice
    return desiredFormat;
}

vk::PresentModeKHR HelloTriangleApplication::ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availableModes) {
    const auto supportsFIFO {
        std::find(availableModes.cbegin(), availableModes.cend(),
            vk::PresentModeKHR::eFifo
        ) != availableModes.cend()
    };
    assert(supportsFIFO && "This device doesn't support our baseline present mode FIFO");

    const auto supportsMailbox {
        std::find(availableModes.cbegin(), availableModes.cend(),
            vk::PresentModeKHR::eMailbox
        ) != availableModes.cend()
    };

    return supportsMailbox? vk::PresentModeKHR::eMailbox: vk::PresentModeKHR::eFifo;
}

vk::Extent2D HelloTriangleApplication::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) {
    // guard: allow the window manager to set our extent on our behalf, which is the
    // usual case and requires us to do nothing
    constexpr auto undefined { std::numeric_limits<uint32_t>::max() };
    if(capabilities.currentExtent.width != undefined) {
        return capabilities.currentExtent;
    }

    // window manager has told us to set our extent ourselves
    int width, height;
    SDL_GetWindowSizeInPixels(mpSDLWindow, &width, &height);

    return {
        std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
        std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
    };
}

uint32_t HelloTriangleApplication::ChooseSwapMinImageCount(const vk::SurfaceCapabilitiesKHR& capabilities) {
    auto minImageCount { std::max(3u, capabilities.minImageCount) };
    if(
        (0 < capabilities.maxImageCount)
        && (capabilities.maxImageCount < minImageCount)
    ) {
        minImageCount = capabilities.maxImageCount;
    }
    return minImageCount;
}

