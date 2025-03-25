#include <stdio.h>
#include <stdlib.h> 
//#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "../my_libs/my_defines.hpp"
#include "../my_libs/my_math.cpp"
#include "../my_libs/string.hpp"
#include "../my_libs/array.hpp"
#include "../my_libs/files.hpp"

#include <windows.h>

// #include "include\raylib.h"
// #include "include\raymath.h"
// #include "include\rlgl.h"

#include "include/Vulkan/vulkan/vulkan.h"

// #include "structs.hpp"

// global_variable Core core = {};

// #include "utils.cpp"
// #include "render.cpp"


int render_width = 1600;
int render_height = 900;
f32 aspect_ratio = 1.0f;

#define SCREEN_WORLD_SIZE 150.0f

// #define UNIT_SIZE (render_width / 150.0f)

//reference 1920x1080 (1500)
#define UI_SCALING ((render_width * 0.5f + render_height * 0.5f) * 0.00066666f) // like / 1500.0f

b32 screen_size_changed = 0;
b32 bordless_fullscreen = false;

b32 window_minimized = false;

HINSTANCE hInstance;
HWND hWnd;

VkInstance create_vulkan_instance() {
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    // Enable required extensions (Win32 needs VK_KHR_win32_surface)
    const char* extensions[] = { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME };
    createInfo.enabledExtensionCount = 2;
    createInfo.ppEnabledExtensionNames = extensions;

    VkInstance instance;
    if (vkCreateInstance(&createInfo, NULL, &instance) != VK_SUCCESS) {
        // throw std::runtime_error("Failed to create Vulkan instance!");
    }
    return instance;
}

VkSurfaceKHR create_win32_surface(VkInstance instance, HWND hwnd) {
    VkWin32SurfaceCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = hwnd;
    createInfo.hinstance = GetModuleHandle(NULL);

    VkSurfaceKHR surface;
    if (vkCreateWin32SurfaceKHR(instance, &createInfo, NULL, &surface) != VK_SUCCESS) {
        // throw std::runtime_error("Failed to create Win32 surface!");
    }
    return surface;
}

VkPhysicalDevice pick_physical_device(VkInstance instance, VkSurfaceKHR surface) {
    u32 deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
    Dynamic_Array<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data);

    for (i32 i = 0; i < devices.count; i++) {
        VkPhysicalDevice *device = devices.get_ptr(i);
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(*device, &props);
        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            // Check queue families (must support graphics and presentation)
            u32 queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(*device, &queueFamilyCount, NULL);
            Dynamic_Array<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(*device, &queueFamilyCount, queueFamilies.data);

            for (u32 i = 0; i < queueFamilyCount; i++) {
                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(*device, i, surface, &presentSupport);
                if (queueFamilies.get(i).queueFlags & VK_QUEUE_GRAPHICS_BIT && presentSupport) {
                    return *device;
                }
            }
        }
    }
    
    printf("ERROR: Could not found physical device.\n");
    return {};
    // throw std::runtime_error("No suitable GPU found!");
}

VkDevice create_logical_device(VkPhysicalDevice physicalDevice, u32& graphicsQueueFamily) {
    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = graphicsQueueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.queueCreateInfoCount = 1;

    // Enable required extensions (swapchain)
    const char* extensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    createInfo.enabledExtensionCount = 1;
    createInfo.ppEnabledExtensionNames = extensions;

    VkDevice device;
    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
        // throw std::runtime_error("Failed to create logical device!");
    }
    return device;
}

VkSwapchainKHR create_swapchain(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, u32 width, u32 height) {
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = capabilities.minImageCount + 1; // +1 for triple buffering
    createInfo.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
    createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    createInfo.imageExtent = { width, height };
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR; // VSync
    createInfo.clipped = VK_TRUE;

    VkSwapchainKHR swapchain;
    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain) != VK_SUCCESS) {
        // throw std::runtime_error("Failed to create swapchain!");
    }
    return swapchain;
}

VkRenderPass create_render_pass(VkDevice device, VkFormat swapchainFormat) {
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = swapchainFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    VkRenderPass renderPass;
    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        // throw std::runtime_error("Failed to create render pass!");
    }
    return renderPass;
}

VkPipeline createGraphicsPipeline(VkDevice device, VkRenderPass renderPass) {
    // Load shaders (read compiled SPIR-V code)
    auto vertShaderCode = readFile("vert.spv");
    auto fragShaderCode = readFile("frag.spv");

    VkShaderModule vertShaderModule = createShaderModule(device, vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(device, fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    // Vertex input (we're hardcoding positions in the shader)
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;

    // Pipeline assembly & rasterization
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // ... (Add viewport, scissor, rasterizer, etc.)

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    // ... (Fill other states)

    pipelineInfo.layout = pipelineLayout; // (You need a VkPipelineLayout)
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;

    VkPipeline pipeline;
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
        // throw std::runtime_error("Failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    vkDestroyShaderModule(device, vertShaderModule, nullptr);

    return pipeline;
}



LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void CreateWindowInstance(HINSTANCE hInstance) {
    const char* CLASS_NAME = "VulkanWindowClass";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    hWnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                    // Window class
        "Vulkan Window",               // Window text
        WS_OVERLAPPEDWINDOW,           // Window style
        CW_USEDEFAULT, CW_USEDEFAULT,  // Size and position
        render_width, render_height,                      // Width and height
        NULL,                          // Parent window
        NULL,                          // Menu
        hInstance,                     // Instance handle
        NULL                           // Additional application data
    );

    if (hWnd == NULL) {
        MessageBox(NULL, "Failed to create window!", "Error", MB_OK);
        exit(1);
    }

    ShowWindow(hWnd, SW_SHOW);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nShowCmd) {
    ::hInstance = hInstance;

    CreateWindowInstance(hInstance);

    // Initialize Vulkan here (not covered in this example)

    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}