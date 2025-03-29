#include <vulkan/vulkan.h>
#include "sdl_marshmallow.h"
#include "pepsimania_math.h"

#include "SDL_vulkan.h"
// @Todo: Remove this evil header and its malicious functions
#include <vector>
#include <cstring>

#define VK_API_VERSION_1_4 VK_MAKE_API_VERSION(0, 1, 4, 0)

const std::vector<const char*> validation_layers = {
    "VK_LAYER_KHRONOS_validation"
};

static VkInstance                   instance;
static VkPhysicalDevice             physical_device = VK_NULL_HANDLE;
static VkDevice                     device;
static VkQueue                      graphics_queue;
static VkSurfaceKHR                 surface;
static VkPipelineLayout             pipeline_layout;
static VkRenderPass                 renderpass;
static VkPipeline                   graphics_pipeline;
static VkCommandPool                command_pool;
static std::vector<VkCommandBuffer> command_buffers;

static VkSwapchainKHR               swapchain;
static VkExtent2D                   swap_extent;
static std::vector<VkImage>         swapchain_images;
static std::vector<VkImageView>     swapchain_image_views;
static std::vector<VkFramebuffer>   swapchain_framebuffers;
static VkFormat                     swapchain_image_format;

std::vector<VkSemaphore>            image_available_semas;
std::vector<VkSemaphore>            render_finished_semas;
std::vector<VkFence>                begin_render_fences;

const int MAX_IMAGES_PER_FRAME = 3;
u32 current_frame_index        = 0;

static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanReportFunc(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT objType,
    uint64_t obj,
    size_t location,
    int32_t code,
    const char* layerPrefix,
    const char* msg,
    void* userData)
{
    printf("VULKAN VALIDATION: %s\n", msg);
    return VK_FALSE;
}

PFN_vkCreateDebugReportCallbackEXT SDL2_vkCreateDebugReportCallbackEXT = nullptr;
void
vk_debug_callback(VkInstance instance)
{
   VkDebugReportCallbackEXT debug_callback;
   SDL2_vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)SDL_Vulkan_GetVkGetInstanceProcAddr();

   VkDebugReportCallbackCreateInfoEXT debug_callback_info = {};
   debug_callback_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
   debug_callback_info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
   debug_callback_info.pfnCallback = VulkanReportFunc;
   SDL2_vkCreateDebugReportCallbackEXT(instance, &debug_callback_info, 0, &debug_callback);
}

static VkShaderModule
vk_create_shader_module (VkDevice device, void *shader_data, u32 binary_size)
{
   VkShaderModule shader_module;
   VkShaderModuleCreateInfo create_info{};

   create_info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
   create_info.codeSize = binary_size;
   create_info.pCode    = (u32*)shader_data;

   if (vkCreateShaderModule(device, &create_info, nullptr, &shader_module) != VK_SUCCESS) {
         printf("[ERROR]: Failed to create shader module\n");
   }

   return shader_module;
}

inline u32
vk_get_image_count(VkSurfaceCapabilitiesKHR capabilities)
{
   u32 image_count = capabilities.minImageCount + 1;
   if ((capabilities.maxImageCount > 0) && (image_count > capabilities.maxImageCount))
      image_count = capabilities.maxImageCount;

   return image_count;
}

std::vector<VkImage>
vk_get_swapchain_images(VkPhysicalDevice physical_device, VkDevice device,
                        VkSurfaceKHR surface, VkSwapchainKHR swapchain)
{
   std::vector<VkImage> new_swapchain_images;
   VkSurfaceCapabilitiesKHR capabilities;

   if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities) != VK_SUCCESS) {
      printf("[ERROR]: Failed to acquire surface capabilities for swapchain creation\n");
      // return false;
   }

   u32 image_count = vk_get_image_count(capabilities);

   vkGetSwapchainImagesKHR(device, swapchain, &image_count, nullptr);
   new_swapchain_images.resize(image_count);
   vkGetSwapchainImagesKHR(device, swapchain, &image_count, new_swapchain_images.data());
   // swapchain_image_format = surface_format.format;
   return new_swapchain_images;
}

static VkSwapchainKHR
vk_create_swapchain(SDL_Window *window,
                    VkPhysicalDevice physical_device, VkDevice device,
                    VkSurfaceKHR surface, VkExtent2D &swap_extent)
{
   VkSwapchainKHR new_swapchain;
   VkSurfaceFormatKHR surface_format;
   VkPresentModeKHR present_mode;
   VkSurfaceCapabilitiesKHR capabilities;
   std::vector<VkSurfaceFormatKHR> formats;
   std::vector<VkPresentModeKHR> present_modes;
   u32 format_count = 0;
   u32 mode_count = 0;

   if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities) != VK_SUCCESS) {
      printf("[ERROR]: Failed to acquire surface capabilities for swapchain creation\n");
      // return false;
   }

   vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr);
   formats.resize(format_count);
   vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, formats.data());

   vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &mode_count, nullptr);
   present_modes.resize(mode_count);
   vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &mode_count, present_modes.data());

   for (int i = 0; i <= format_count; ++i) {
      if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
         surface_format = formats[i];
         break;
      }
   }

   for (int i = 0; i <= mode_count; ++i) {
      if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
          present_mode = present_modes[i];
         break;
      }
   }

   if (present_mode == NULL)
         present_mode = VK_PRESENT_MODE_FIFO_KHR;

   // Some window managers allow us to set the window extent so we need to query the current frame buffer for the width and heigh
   // and set the extent that way.
   if (capabilities.currentExtent.width != UINT_MAX) {
      swap_extent = capabilities.currentExtent;
   } else {
      int w, h;
      SDL_GetWindowSize(window, &w, &h);
      VkExtent2D new_extent = {(u32)w, (u32)h};

      new_extent.width  = clamp(new_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
      new_extent.width  = clamp(new_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
      swap_extent       = new_extent;
   }

   u32 image_count = vk_get_image_count(capabilities);

   VkSwapchainCreateInfoKHR swapchain_create_info{};
   swapchain_create_info.sType                   = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
   swapchain_create_info.pNext                   = NULL;
   swapchain_create_info.surface                 = surface;
   swapchain_create_info.minImageCount           = image_count;
   swapchain_create_info.imageFormat             = surface_format.format;
   swapchain_create_info.imageColorSpace         = surface_format.colorSpace;
   swapchain_create_info.imageExtent             = swap_extent;
   swapchain_create_info.imageArrayLayers        = 1;
   swapchain_create_info.imageUsage              = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
   swapchain_create_info.imageSharingMode        = VK_SHARING_MODE_EXCLUSIVE;
   swapchain_create_info.queueFamilyIndexCount   = 0;
   swapchain_create_info.pQueueFamilyIndices     = nullptr;
   swapchain_create_info.preTransform            = capabilities.currentTransform;
   swapchain_create_info.compositeAlpha          = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
   swapchain_create_info.presentMode             = present_mode;
   swapchain_create_info.clipped                 = VK_TRUE;
   swapchain_create_info.oldSwapchain            = VK_NULL_HANDLE; //@Todo: Null handle until swapchain recreation is online

   if (vkCreateSwapchainKHR(device, &swapchain_create_info, nullptr, &new_swapchain) != VK_SUCCESS) {
      printf("[ERROR]: Failed to create swapchain\n");
      // return false;
   }

   // @Todo: Assign this outside of the function or maybe send it in as a pointer?
   swapchain_image_format = surface_format.format;
   return new_swapchain;
}

std::vector<VkImageView>
vk_create_swapchain_image_views(VkDevice device, std::vector<VkImage> swapchain_images)
{
   std::vector<VkImageView> new_swapchain_image_views;

   new_swapchain_image_views.resize(swapchain_images.size());
   for (u32 i = 0; i < new_swapchain_image_views.size(); ++i) {
      VkImageViewCreateInfo view_create_info{};
      view_create_info.sType                       = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      view_create_info.pNext                       = NULL;
      view_create_info.image                       = swapchain_images[i];
      view_create_info.viewType                    = VK_IMAGE_VIEW_TYPE_2D;
      view_create_info.format                      = swapchain_image_format;
      view_create_info.components.r                = VK_COMPONENT_SWIZZLE_IDENTITY;
      view_create_info.components.g                = VK_COMPONENT_SWIZZLE_IDENTITY;
      view_create_info.components.b                = VK_COMPONENT_SWIZZLE_IDENTITY;
      view_create_info.components.a                = VK_COMPONENT_SWIZZLE_IDENTITY;
      view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      view_create_info.subresourceRange.levelCount = 1;
      view_create_info.subresourceRange.layerCount = 1;
      view_create_info.subresourceRange.baseMipLevel = 0;
      view_create_info.subresourceRange.baseArrayLayer = 0;

      if (vkCreateImageView(device, &view_create_info, nullptr, &new_swapchain_image_views[i]) != VK_SUCCESS) {
         printf("[ERROR]: Failed to create image views\n");
         // return false;
      }
   }

   return new_swapchain_image_views;
}

std::vector<VkFramebuffer>
vk_create_framebuffers(std::vector<VkImageView> swapchain_image_view, VkRenderPass renderpass, VkExtent2D swap_extent)
{
   std::vector<VkFramebuffer> framebuffers;
   framebuffers.resize(swapchain_image_views.size());
   for (u32 i = 0; i < swapchain_image_views.size(); ++i) {
      VkImageView attachments[] = {
         swapchain_image_views[i]
      };

       VkFramebufferCreateInfo framebuffer_info{};
       framebuffer_info.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
       framebuffer_info.renderPass      = renderpass;
       framebuffer_info.attachmentCount = 1;
       framebuffer_info.pAttachments    = attachments;
       framebuffer_info.width           = swap_extent.width;
       framebuffer_info.height          = swap_extent.height;
       framebuffer_info.layers          = 1;

      if (vkCreateFramebuffer(device, &framebuffer_info, nullptr, &framebuffers[i]) != VK_SUCCESS) {
         printf("[ERROR]: Failed to create framebuffer\n");
         // return false;
      }
   }

   return framebuffers;
}

static void
vk_kill_swpachain()
{
   for (int i = 0; i < swapchain_framebuffers.size(); ++i) {
      vkDestroyFramebuffer(device, swapchain_framebuffers[i], nullptr);
   }
   for (int i = 0; i < swapchain_image_views.size(); ++i) {
      vkDestroyImageView(device, swapchain_image_views[i], nullptr);
   }
   vkDestroySwapchainKHR(device, swapchain, nullptr);
}

static void
vk_recreate_swapchain(SDL_Window *window)
{
   vkDeviceWaitIdle(device);
   vk_kill_swpachain();
   swapchain               = vk_create_swapchain(window, physical_device, device, surface, swap_extent);
   swapchain_images        = vk_get_swapchain_images(physical_device, device, surface, swapchain);
   swapchain_image_views   = vk_create_swapchain_image_views(device, swapchain_images);
   swapchain_framebuffers  = vk_create_framebuffers(swapchain_image_views, renderpass, swap_extent);
}


bool
vulkan_init(SDL_Window *window)
{
   bool success = false;
#if NDEBUG
   const bool enable_validation_layer = false;
#else
   const bool enable_validation_layer = true;
#endif

   VkApplicationInfo app_info  = {};
   app_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
   app_info.pApplicationName   = "Marshmallow";
   app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
   app_info.pEngineName        = "Marshmallow";
   app_info.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
   app_info.apiVersion         = VK_API_VERSION_1_4;

   u32 extension_count = 0;
   // const char *extension_names;
   success = SDL_Vulkan_GetInstanceExtensions(window,
                                              &extension_count,
                                              NULL);
   std::vector<const char *>extension_names(extension_count);

   success = SDL_Vulkan_GetInstanceExtensions(window,
                                              &extension_count,
                                              extension_names.data());

   VkInstanceCreateInfo instance_info     = {};
   instance_info.sType                    = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
   instance_info.pApplicationInfo         = &app_info;
   instance_info.enabledExtensionCount    = extension_count;
   instance_info.ppEnabledExtensionNames  = extension_names.data();
   instance_info.enabledLayerCount        = 0;

   {
      u32 layer_count = 0;
      vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
      std::vector<VkLayerProperties> layers_available(layer_count);
      vkEnumerateInstanceLayerProperties(&layer_count, layers_available.data());

      bool layer_found = false;
      for (auto current_layer : layers_available) {
         // Just checking the one instance layer for now
         if (strcmp(validation_layers[0], current_layer.layerName)) {
            layer_found = true;
            break;
         }
      }

      if (layer_found) {
         instance_info.enabledLayerCount     = (u32)validation_layers.size();
         instance_info.ppEnabledLayerNames   = validation_layers.data();
      }
   }

   VkResult res = vkCreateInstance(&instance_info, nullptr, &instance);

   if (enable_validation_layer)
      vk_debug_callback(instance);

   // Surface creation
   success = SDL_Vulkan_CreateSurface(window,
                                      instance,
                                      &surface);

   u32 graphics_family_index = 0;
   u32 present_family_index = 0; // @Todo: Check if the physical device and queue supports graphics and present queues seperately

   // Physical Devices and Queue Family init
   {
      u32 device_count = 0;
      vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
      if (device_count == 0) {
         printf("[ERROR]: Failed to find any GPUs that can run vulkan on this machine\n");
         return false;
      }

      std::vector<VkPhysicalDevice> devices(device_count);
      vkEnumeratePhysicalDevices(instance, &device_count, devices.data());
      if (device_count = 1)
         physical_device = devices[0];

      VkPhysicalDeviceProperties device_properties{};
      vkGetPhysicalDeviceProperties(physical_device, &device_properties);
      if (physical_device) {
         // @Todo: Convert the version numbers into something readable
         printf("Vulkan API Version: %u\n",     device_properties.apiVersion);
         printf("Vulkan Driver Version: %u\n",  device_properties.driverVersion);
         printf("Vulkan Vendor ID %u\n",        device_properties.vendorID);
         printf("Vulkan Device ID: %u\n",       device_properties.deviceID);
         printf("Vulkan Device Name: %s\n",     device_properties.deviceName);
      }

      u32 queue_family_count = 0;
      VkQueueFamilyProperties queue_family_properties{};

      vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, NULL);
      std::vector<VkQueueFamilyProperties> queue_family_array(queue_family_count);
      vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_family_array.data());

      for (u32 i = 0; i < queue_family_count; ++i) {
         if (queue_family_array[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphics_family_index = i;
         }

         VkBool32 present_support = false;
         vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &present_support);
         if (present_support) {
            present_family_index = i;
         }
      }

   }

   // Logical Device init
   {
      std::vector<const char*> device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
      VkPhysicalDeviceFeatures device_features{};

      float queue_priority = 1.0;
      VkDeviceQueueCreateInfo device_queue_info = {};
      device_queue_info.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      device_queue_info.pNext                   = NULL;
      device_queue_info.queueFamilyIndex        = graphics_family_index;
      device_queue_info.queueCount              = 1;
      device_queue_info.pQueuePriorities        = &queue_priority;

      VkDeviceCreateInfo device_create_info     = {};
      device_create_info.sType                  = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
      device_create_info.pNext                  = NULL;
      device_create_info.queueCreateInfoCount   = 1;
      device_create_info.pQueueCreateInfos      = &device_queue_info;
      device_create_info.pEnabledFeatures       = &device_features;
      device_create_info.enabledExtensionCount  = 1;
      device_create_info.ppEnabledExtensionNames = device_extensions.data();

      if (vkCreateDevice(physical_device, &device_create_info, NULL, &device) != VK_SUCCESS) {
         printf("[ERROR]: Failed to initialize device\n");
         return false;
      }

      vkGetDeviceQueue(device, graphics_family_index, 0, &graphics_queue);
   }

   swapchain             = vk_create_swapchain(window, physical_device, device, surface, swap_extent);
   swapchain_images      = vk_get_swapchain_images(physical_device, device, surface, swapchain);
   swapchain_image_views = vk_create_swapchain_image_views(device, swapchain_images);

   // Renderpass and subpass init
   {
      VkAttachmentDescription color_attachment{};
      color_attachment.format = swapchain_image_format;
      color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
      color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
      color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
      color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

      VkAttachmentReference color_attachment_ref{};
      color_attachment_ref.attachment = 0;
      color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

      VkSubpassDescription subpass{};
      subpass.pipelineBindPoint     = VK_PIPELINE_BIND_POINT_GRAPHICS;
      subpass.colorAttachmentCount  = 1;
      subpass.pColorAttachments     = &color_attachment_ref;

      VkSubpassDependency dependency = {};
      dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
      dependency.dstSubpass = 0;
      dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      dependency.srcAccessMask = 0;
      dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

      VkRenderPassCreateInfo render_pass_info{};
      render_pass_info.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
      render_pass_info.attachmentCount = 1;
      render_pass_info.pAttachments    = &color_attachment;
      render_pass_info.subpassCount    = 1;
      render_pass_info.pSubpasses      = &subpass;
      render_pass_info.dependencyCount = 1;
      render_pass_info.pDependencies   = &dependency;

      if (vkCreateRenderPass(device, &render_pass_info, nullptr, &renderpass) != VK_SUCCESS) {
          printf("[ERROR]: Failed to create render pass\n");
          return false;
      }
   }

   // Graphics Pipeline creation stuff
   {
      MarshFile vertex_shader_file  = debug_read_file("C:/marshmallow/shaders/vert.spv");
      MarshFile frag_shader_file    = debug_read_file("C:/marshmallow/shaders/frag.spv");

      VkShaderModule vertex_module  = vk_create_shader_module(device, vertex_shader_file.contents, vertex_shader_file.size);
      VkShaderModule frag_module    = vk_create_shader_module(device, frag_shader_file.contents, frag_shader_file.size);

      VkPipelineShaderStageCreateInfo vertex_stage_info  = {};
      vertex_stage_info.sType                            = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      vertex_stage_info.pNext                            = NULL;
      vertex_stage_info.stage                            = VK_SHADER_STAGE_VERTEX_BIT;
      vertex_stage_info.module                           = vertex_module;
      vertex_stage_info.pName                            = "main";
      vertex_stage_info.pSpecializationInfo              = NULL;

      VkPipelineShaderStageCreateInfo frag_stage_info = {};
      frag_stage_info.sType                           = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      frag_stage_info.pNext                           = NULL;
      frag_stage_info.stage                           = VK_SHADER_STAGE_FRAGMENT_BIT;
      frag_stage_info.module                          = frag_module;
      frag_stage_info.pName                           = "main";
      frag_stage_info.pSpecializationInfo             = NULL;

      VkPipelineShaderStageCreateInfo shader_stages[] = {vertex_stage_info, frag_stage_info};

      std::vector<VkDynamicState> dynamic_states_settings = {
         VK_DYNAMIC_STATE_VIEWPORT,
         VK_DYNAMIC_STATE_SCISSOR
      };
      VkPipelineDynamicStateCreateInfo dynamic_state = {};
      dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
      dynamic_state.pNext = NULL;
      dynamic_state.dynamicStateCount = 2;
      dynamic_state.pDynamicStates = dynamic_states_settings.data();

      VkPipelineVertexInputStateCreateInfo vertex_input_info{};
      vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
      vertex_input_info.vertexBindingDescriptionCount = 0;
      vertex_input_info.pVertexBindingDescriptions = nullptr; // Optional
      vertex_input_info.vertexAttributeDescriptionCount = 0;
      vertex_input_info.pVertexAttributeDescriptions = nullptr; // Optional

      VkPipelineInputAssemblyStateCreateInfo input_assembly{};
      input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
      input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
      input_assembly.primitiveRestartEnable = VK_FALSE;

      VkViewport viewport{};
      viewport.x = 0.0f;
      viewport.y = 0.0f;
      viewport.width = (float)swap_extent.width;
      viewport.height = (float)swap_extent.height;
      viewport.minDepth = 0.0f;
      viewport.maxDepth = 1.0f;

      VkRect2D scissor{};
      scissor.offset = {0, 0};
      scissor.extent = swap_extent;

      VkPipelineViewportStateCreateInfo viewport_state{};
      viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
      viewport_state.viewportCount = 1;
      viewport_state.scissorCount = 1;

      VkPipelineRasterizationStateCreateInfo rasterizer_state{};
      rasterizer_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
      rasterizer_state.depthClampEnable = VK_FALSE;
      rasterizer_state.rasterizerDiscardEnable = VK_FALSE;
      rasterizer_state.polygonMode = VK_POLYGON_MODE_FILL;
      rasterizer_state.lineWidth = 1.0f;
      rasterizer_state.cullMode = VK_CULL_MODE_BACK_BIT;
      rasterizer_state.frontFace = VK_FRONT_FACE_CLOCKWISE;
      rasterizer_state.depthBiasEnable = VK_FALSE;
      rasterizer_state.depthBiasConstantFactor = 0.0f; // Optional
      rasterizer_state.depthBiasClamp = 0.0f; // Optional
      rasterizer_state.depthBiasSlopeFactor = 0.0f; // Optional

      VkPipelineMultisampleStateCreateInfo multisampling_state{};
      multisampling_state.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
      multisampling_state.sampleShadingEnable = VK_FALSE;
      multisampling_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
      multisampling_state.minSampleShading = 1.0f; // Optional
      multisampling_state.pSampleMask = nullptr; // Optional
      multisampling_state.alphaToCoverageEnable = VK_FALSE; // Optional
      multisampling_state.alphaToOneEnable = VK_FALSE; // Optional

      VkPipelineDepthStencilStateCreateInfo depth_stencil_state = {};

      VkPipelineColorBlendAttachmentState color_blend_attachment{};
      color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
      color_blend_attachment.blendEnable = VK_FALSE;
      color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
      color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
      color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
      color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
      color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
      color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

      VkPipelineColorBlendStateCreateInfo color_blending_state{};
      color_blending_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
      color_blending_state.logicOpEnable = VK_FALSE;
      color_blending_state.logicOp = VK_LOGIC_OP_COPY; // Optional
      color_blending_state.attachmentCount = 1;
      color_blending_state.pAttachments = &color_blend_attachment;
      color_blending_state.blendConstants[0] = 0.0f; // Optional
      color_blending_state.blendConstants[1] = 0.0f; // Optional
      color_blending_state.blendConstants[2] = 0.0f; // Optional
      color_blending_state.blendConstants[3] = 0.0f; // Optional

      VkPipelineLayoutCreateInfo pipeline_layout_info{};
      pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
      pipeline_layout_info.setLayoutCount = 0; // Optional
      pipeline_layout_info.pSetLayouts = nullptr; // Optional
      pipeline_layout_info.pushConstantRangeCount = 0; // Optional
      pipeline_layout_info.pPushConstantRanges = nullptr; // Optional

      if (vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr, &pipeline_layout) != VK_SUCCESS) {
          printf("[ERROR]: Failed to create pipeline layout\n");
          return false;
      }

      VkGraphicsPipelineCreateInfo pipeline_info{};
      pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
      pipeline_info.stageCount = 2;
      pipeline_info.pStages = shader_stages;
      pipeline_info.pVertexInputState = &vertex_input_info;
      pipeline_info.pInputAssemblyState = &input_assembly;
      pipeline_info.pViewportState = &viewport_state;
      pipeline_info.pRasterizationState = &rasterizer_state;
      pipeline_info.pMultisampleState = &multisampling_state;
      pipeline_info.pDepthStencilState = nullptr; // Optional
      pipeline_info.pColorBlendState = &color_blending_state;
      pipeline_info.pDynamicState = &dynamic_state;
      pipeline_info.layout = pipeline_layout;
      pipeline_info.renderPass = renderpass;
      pipeline_info.subpass = 0;
      pipeline_info.basePipelineHandle = VK_NULL_HANDLE; // Optional
      pipeline_info.basePipelineIndex = -1; // Optional

      if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &graphics_pipeline) != VK_SUCCESS) {
           printf("[ERROR]: Failed to create graphics pipeline\n");
           return false;
      }

   }

   swapchain_framebuffers = vk_create_framebuffers(swapchain_image_views, renderpass, swap_extent);

   // Create command pools init
   {
      VkCommandPoolCreateInfo pool_info{};
      pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
      pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
      pool_info.queueFamilyIndex = graphics_family_index;

      if (vkCreateCommandPool(device, &pool_info, nullptr, &command_pool) != VK_SUCCESS) {
          printf("[ERROR]: Failed to create command pool\n");
          return false;
      }

   // Create command buffers init
      command_buffers.resize(MAX_IMAGES_PER_FRAME);
      VkCommandBufferAllocateInfo alloc_info  = {};
      alloc_info.sType                        = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      alloc_info.commandPool                  = command_pool;
      alloc_info.level                        = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
      alloc_info.commandBufferCount           = command_buffers.size();
      if (vkAllocateCommandBuffers(device, &alloc_info, command_buffers.data()) != VK_SUCCESS) {
         printf("[ERROR] Failed to allocate command buffers\n");
         return false;
      }

      bool poo = true;
   }

   image_available_semas.resize(MAX_IMAGES_PER_FRAME);
   render_finished_semas.resize(MAX_IMAGES_PER_FRAME);
   begin_render_fences.resize(MAX_IMAGES_PER_FRAME);

   VkSemaphoreCreateInfo semaphore_info{};
   semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

   VkFenceCreateInfo fence_info{};
   fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
   fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

   for (int i = 0; i < MAX_IMAGES_PER_FRAME; ++i) {
      if (vkCreateSemaphore(device, &semaphore_info, nullptr, &image_available_semas[i])   != VK_SUCCESS ||
          vkCreateSemaphore(device, &semaphore_info, nullptr, &render_finished_semas[i])   != VK_SUCCESS ||
          vkCreateFence(device, &fence_info, nullptr, &begin_render_fences[i])             != VK_SUCCESS) {
          printf("[ERROR]: Failed to create semaphores and fences\n");
      }
   }

   return true;
}

static void
vk_record_command_buffer(VkCommandBuffer command_buffer, VkRenderPass renderpass,
                        VkExtent2D swap_extent, VkPipeline graphics_pipeline,
                        std::vector<VkFramebuffer> swapchain_framebuffers, u32 image_index)
{
   VkCommandBufferBeginInfo begin_info = {};
   begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
   begin_info.flags = 0; // Optional
   begin_info.pInheritanceInfo = nullptr; // Optional

   if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS) {
       printf("[ERROR]: Failed to begin recording command buffer\n");
   }

   VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};

   VkRenderPassBeginInfo render_pass_info    = {};
   render_pass_info.sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
   render_pass_info.renderPass               = renderpass;
   render_pass_info.framebuffer              = swapchain_framebuffers[image_index];
   render_pass_info.renderArea.offset        = {0, 0};
   render_pass_info.renderArea.extent        = swap_extent;
   render_pass_info.clearValueCount          = 1;
   render_pass_info.pClearValues             = &clear_color;

   vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
   vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);

   VkViewport viewport   = {};
   viewport.x            = 0.0f;
   viewport.y            = 0.0f;
   viewport.width        = (float)(swap_extent.width);
   viewport.height       = (float)(swap_extent.height);
   viewport.minDepth     = 0.0f;
   viewport.maxDepth     = 1.0f;
   vkCmdSetViewport(command_buffer, 0, 1, &viewport);

   VkRect2D scissor = {};
   scissor.offset = {0, 0};
   scissor.extent = swap_extent;
   vkCmdSetScissor(command_buffer, 0, 1, &scissor);

   vkCmdDraw(command_buffer, 3, 1, 0, 0);
   vkCmdEndRenderPass(command_buffer);

   if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
      printf("[ERROR]: Failed to record command buffer\n");
   }
}

void
vk_draw_frame(SDL_Window *window)
{
   u32 image_index;

   vkWaitForFences(device, 1, &begin_render_fences[current_frame_index], VK_TRUE, UINT64_MAX);
   VkResult res = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, image_available_semas[current_frame_index], VK_NULL_HANDLE, &image_index);

   if (res == VK_ERROR_OUT_OF_DATE_KHR) {
      vk_recreate_swapchain(window);
      return;
   } else if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR) {
      printf("[ERROR]: Failed to present the swapchain image\n");
   }

   vkResetFences(device, 1, &begin_render_fences[current_frame_index]);

   vkResetCommandBuffer(command_buffers[current_frame_index], 0);
   vk_record_command_buffer(command_buffers[current_frame_index], renderpass, swap_extent, graphics_pipeline, swapchain_framebuffers, image_index);

   VkSemaphore wait_semaphores[]       = {image_available_semas[current_frame_index]};
   VkPipelineStageFlags wait_stages[]  = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
   VkSemaphore signal_semaphores[]     = {render_finished_semas[current_frame_index]};

   VkSubmitInfo submit_info          = {};
   submit_info.sType                 = VK_STRUCTURE_TYPE_SUBMIT_INFO;
   submit_info.waitSemaphoreCount    = 1;
   submit_info.pWaitSemaphores       = wait_semaphores;
   submit_info.signalSemaphoreCount  = 1;
   submit_info.pSignalSemaphores     = signal_semaphores;
   submit_info.pWaitDstStageMask     = wait_stages;
   submit_info.commandBufferCount    = 1;
   submit_info.pCommandBuffers       = &command_buffers[current_frame_index];

   if (vkQueueSubmit(graphics_queue, 1, &submit_info, begin_render_fences[current_frame_index]) != VK_SUCCESS) {
      printf("[ERROR] Failed to submit draw command buffer\n");
   }

   VkSwapchainKHR swapchains[] = {swapchain};

   VkPresentInfoKHR present_info    = {};
   present_info.sType               = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
   present_info.waitSemaphoreCount  = 1;
   present_info.pWaitSemaphores     = signal_semaphores;
   present_info.pResults            = nullptr; // Optional
   present_info.swapchainCount      = 1;
   present_info.pSwapchains         = swapchains;
   present_info.pImageIndices       = &image_index;

   res = vkQueuePresentKHR(graphics_queue, &present_info);

   if (res == VK_ERROR_OUT_OF_DATE_KHR) {
      vk_recreate_swapchain(window);
   } else if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR) {
      printf("[ERROR]: Failed to present the swapchain image\n");
   }

   current_frame_index = (current_frame_index + 1) % MAX_IMAGES_PER_FRAME;
}

void vk_cleanup()
{
   for (int i = 0; i < MAX_IMAGES_PER_FRAME; ++i) {
      vkDestroySemaphore(device, image_available_semas[i], nullptr);
      vkDestroySemaphore(device, render_finished_semas[i], nullptr);
      vkDestroyFence(device, begin_render_fences[i], nullptr);
   }
}