#include "testing.h"

#include "general/platformlib/platform/platform.h"


#include <vulkan/vulkan.h>
#define CHECK_VK_RESULT(result) \
    if (result != VK_SUCCESS) { \
        printf("Vulkan error: %d\n", result); \
        exit(1); \
    }
void initVulkan() {
    VkInstance instance;
    VkDevice device;
    VkPhysicalDevice physicalDevice;
    VkQueue graphicsQueue, computeQueue, transferQueue;

    // Initialize Vulkan instance
    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Vulkan Queue Setup",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_3
    };

    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo
    };

    CHECK_VK_RESULT(vkCreateInstance(&createInfo, 0, &instance));

    // Pick a physical device (GPU)
    uint32_t deviceCount = 0;
    CHECK_VK_RESULT(vkEnumeratePhysicalDevices(instance, &deviceCount, 0));

    VkPhysicalDevice* devices = malloc(sizeof(VkPhysicalDevice) * deviceCount);
    CHECK_VK_RESULT(vkEnumeratePhysicalDevices(instance, &deviceCount, devices));
    physicalDevice = devices[0];  // Use the first device

    // Query the queue families
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, 0);
    VkQueueFamilyProperties* queueFamilies = malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies);

    // Find suitable queue families
    int graphicsFamily = -1;
    int computeFamily = -1;
    int transferFamily = -1;

    for (uint32_t i = 0; i < queueFamilyCount; ++i) {
        VkQueueFlags flags = queueFamilies[i].queueFlags;

        if (flags & VK_QUEUE_GRAPHICS_BIT) {
            graphicsFamily = i;  // Found a graphics queue
        }
        if (flags & VK_QUEUE_COMPUTE_BIT) {
            computeFamily = i;  // Found a compute queue
        }
        if (flags & VK_QUEUE_TRANSFER_BIT) {
            transferFamily = i;  // Found a transfer queue
        }
    }

    free(queueFamilies);

    if (graphicsFamily == -1 || computeFamily == -1 || transferFamily == -1) {
        printf("Error: Could not find a suitable queue family for graphics, compute, or transfer\n");
        exit(1);
    }

    // Create logical device and queues
    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfos[3] = {};

    queueCreateInfos[0] = (VkDeviceQueueCreateInfo){
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = graphicsFamily,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority
    };
    queueCreateInfos[1] = (VkDeviceQueueCreateInfo){
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = computeFamily,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority
    };
    queueCreateInfos[2] = (VkDeviceQueueCreateInfo){
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = transferFamily,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority
    };

    VkDeviceCreateInfo deviceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = 3,
        .pQueueCreateInfos = queueCreateInfos
    };

    CHECK_VK_RESULT(vkCreateDevice(physicalDevice, &deviceCreateInfo, 0, &device));

    // Retrieve the queues
    vkGetDeviceQueue(device, graphicsFamily, 0, &graphicsQueue);
    vkGetDeviceQueue(device, computeFamily, 0, &computeQueue);
    vkGetDeviceQueue(device, transferFamily, 0, &transferQueue);

    // Print out the device and queue information
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
    printf("Device: %s\n", deviceProperties.deviceName);
    printf("Graphics Queue Family: %d\n", graphicsFamily);
    printf("Compute Queue Family: %d\n", computeFamily);
    printf("Transfer Queue Family: %d\n", transferFamily);

    // Cleanup
    vkDestroyDevice(device, 0);
    vkDestroyInstance(instance, 0);
    free(devices);
}


int main(int argc, char* argv[]) {

	platform_init();

    show_console_window();

    initVulkan();

	platform_exit();

	return 0;
}