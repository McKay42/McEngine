//================ Copyright (c) 2016, PG, All rights reserved. =================//
//
// Purpose:		vulkan implementation of Graphics
//
// $NoKeywords: $vkg
//===============================================================================//

#include "VulkanGraphicsInterface.h"
#include "Engine.h"

#include "WinEnvironment.h"
#include "LinuxEnvironment.h"

// TODO: implement VulkanGraphicsInterface

VulkanGraphicsInterface::VulkanGraphicsInterface()
{
	if (!vulkan->isReady())
	{
		engine->showMessageErrorFatal("Fatal Vulkan Error", "Couldn't initialize VulkanGraphicsInterface because VulkanInterface is not ready!");
		return;
	}

#ifdef MCENGINE_FEATURE_VULKAN

	// Vulkan has started up successfully, now try to get a surface
	VkResult res;

	// OS specific surface selection
#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo;
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.hinstance = ((WinEnvironment*)(env))->getHInstance(); // provided by the platform code
	surfaceCreateInfo.hwnd = ((WinEnvironment*)(env))->getHwnd(); // provided by the platform code
	res = vkCreateWin32SurfaceKHR(vulkan->getInstance(), &surfaceCreateInfo, NULL, &m_surface);
#else
	#error "TODO: add correct logic here"
#endif
	if (res != VK_SUCCESS)
	{
		engine->showMessageErrorFatal("Fatal Vulkan Error", UString::format("Couldn't vkCreate*SurfaceKHR(), returned %d", res));
		return;
	}

	PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR = NULL;
	*(void **)&vkGetPhysicalDeviceSurfaceFormatsKHR = vkGetInstanceProcAddr(vulkan->getInstance(), "vkGetPhysicalDeviceSurfaceFormatsKHR");
	if (!vkGetPhysicalDeviceSurfaceFormatsKHR)
	{
		engine->showMessageErrorFatal("Fatal Vulkan Error", UString::format("Couldn't vkGetInstanceProcAddr(\"vkGetPhysicalDeviceSurfaceFormatsKHR\"), returned %d.", res));
		return;
	}

	// set surface properties and formats
	unsigned int formatCount = 0;
	res = vkGetPhysicalDeviceSurfaceFormatsKHR(vulkan->getPhysicalDevice(), m_surface, &formatCount, NULL);

	if (formatCount < 1)
	{
		engine->showMessageErrorFatal("Fatal Vulkan Error", UString::format("Couldn't vkGetPhysicalDeviceSurfaceFormatsKHR(), returned %d", res));
		return;
	}

	std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
	res = vkGetPhysicalDeviceSurfaceFormatsKHR(vulkan->getPhysicalDevice(), m_surface, &formatCount, &surfaceFormats[0]);

	if (formatCount == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED)
		m_colorFormat = VK_FORMAT_B8G8R8A8_UNORM;
	else
	{
		debugLog("Vulkan: Multiple surface formats detected, selecting #%i as default format (%i).\n", 0, (int)surfaceFormats[0].format);
	    m_colorFormat = surfaceFormats[0].format;
	}
	m_colorSpace = surfaceFormats[0].colorSpace;


	//*********************************************************************************************************************************************************//

	fpGetPhysicalDeviceSurfaceSupportKHR = swapChainGetInstanceProc("vkGetPhysicalDeviceSurfaceSupportKHR");
	fpGetPhysicalDeviceSurfaceCapabilitiesKHR = swapChainGetInstanceProc("vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
	fpGetPhysicalDeviceSurfaceFormatsKHR = swapChainGetInstanceProc("vkGetPhysicalDeviceSurfaceFormatsKHR");
	fpGetPhysicalDeviceSurfacePresentModesKHR = swapChainGetInstanceProc("vkGetPhysicalDeviceSurfacePresentModesKHR");

	fpCreateSwapchainKHR = swapChainGetDeviceProc("vkCreateSwapchainKHR");
	fpDestroySwapchainKHR = swapChainGetDeviceProc("vkDestroySwapchainKHR");
	fpGetSwapchainImagesKHR = swapChainGetDeviceProc("vkGetSwapchainImagesKHR");
	fpAcquireNextImageKHR = swapChainGetDeviceProc("vkAcquireNextImageKHR");
	fpQueuePresentKHR = swapChainGetDeviceProc("vkQueuePresentKHR");

	//*********************************************************************************************************************************************************//

	// swap chain creation

	uint32_t queueCount;
	vkGetPhysicalDeviceQueueFamilyProperties(vulkan->getPhysicalDevice(), &queueCount, NULL);
	if (queueCount < 1)
	{
		engine->showMessageErrorFatal("Fatal Vulkan Error", "Couldn't vkGetPhysicalDeviceQueueFamilyProperties()!");
		return;
	}

	std::vector<VkQueueFamilyProperties> queueProperties(queueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(vulkan->getPhysicalDevice(), &queueCount, &queueProperties[0]);

	// In previous tutorials we just picked which ever queue was readily
	// available. The problem is not all queues support presenting. Here
	// we make use of vkGetPhysicalDeviceSurfaceSupportKHR to find queues
	// with present support.
	std::vector<VkBool32> supportsPresent(queueCount);
	for (uint32_t i=0; i<queueCount; i++)
	{
		fpGetPhysicalDeviceSurfaceSupportKHR(vulkan->getPhysicalDevice(), i, m_surface, &supportsPresent[i]);
	}

	// Now we have a list of booleans for which queues support presenting.
	// We now must walk the queue to find one which supports
	// VK_QUEUE_GRAPHICS_BIT and has supportsPresent[index] == VK_TRUE
	// (indicating it supports both.)
	uint32_t graphicIndex = 0xffffffff;
	uint32_t presentIndex = 0xffffffff;
	for (uint32_t i=0; i<queueCount; i++)
	{
		if ((queueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
		{
			if (graphicIndex == 0xffffffff)
				graphicIndex = i;

			if (supportsPresent[i] == VK_TRUE)
			{
				graphicIndex = i;
				presentIndex = i;
				break;
			}
		}
	}

	if (graphicIndex == 0xffffffff || presentIndex == 0xffffffff)
	{
		engine->showMessageErrorFatal("Fatal Vulkan Error", "Couldn't find a suitable queue for rendering and presenting!");
		return;
	}

	m_nodeIndex = graphicIndex;

	// Get physical device surface properties and formats. This was not
	// covered in previous tutorials. Effectively does what it says it does.
	// We will be using the result of this to determine the number of
	// images we should use for our swap chain and set the appropriate
	// sizes for them.
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	fpGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkan->getPhysicalDevice(), m_surface, &surfaceCapabilities);

	uint32_t presentModeCount;
	fpGetPhysicalDeviceSurfacePresentModesKHR(vulkan->getPhysicalDevice(), m_surface, &presentModeCount, NULL);

	if (presentModeCount < 1)
	{
		engine->showMessageErrorFatal("Fatal Vulkan Error", "presentModeCount < 1!");
		return;
	}

	std::vector<VkPresentModeKHR> presentModes(presentModeCount);
	fpGetPhysicalDeviceSurfacePresentModesKHR(vulkan->getPhysicalDevice(), m_surface, &presentModeCount, &presentModes[0]);


	// When constructing a swap chain we must supply our surface resolution.
	// Like all things in Vulkan there is a structure for representing this
	VkExtent2D swapChainExtent = { };

	// The way surface capabilities work is rather confusing but width
	// and height are either both -1 or both not -1. A size of -1 indicates
	// that the surface size is undefined, which means you can set it to
	// effectively any size. If the size however is defined, the swap chain
	// size *MUST* match.
	uint32_t screenWidth = engine->getScreenWidth();
	uint32_t screenHeight = engine->getScreenWidth();
	if (surfaceCapabilities.currentExtent.width == -1)
	{
		swapChainExtent.width = screenWidth;
		swapChainExtent.height = screenHeight;
	}
	else
	{
		swapChainExtent = surfaceCapabilities.currentExtent;
		screenWidth = swapChainExtent.width;
		screenHeight = swapChainExtent.height;
		debugLog("Vulkan: Surface resolution is limited/set to %ix%i\n", screenWidth, screenHeight);
	}


	// swap mode (vsync, tearing etc)
	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR; // VK_PRESENT_MODE_FIFO_KHR is always supported

	// Determine the number of images for our swap chain
	uint32_t desiredImages = surfaceCapabilities.minImageCount + 1;
	if (surfaceCapabilities.maxImageCount > 0 &&  desiredImages > surfaceCapabilities.maxImageCount)
	{
		desiredImages = surfaceCapabilities.maxImageCount;
	}

	debugLog("Vulkan: Using %i images for swap chain\n", desiredImages);


	VkSurfaceTransformFlagsKHR preTransform = surfaceCapabilities.currentTransform;
	if (surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;


	// actually create the swap chain
	VkSwapchainCreateInfoKHR swapChainCreateInfo = { };

	// Mandatory fields
	swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChainCreateInfo.pNext = NULL;

	swapChainCreateInfo.surface = m_surface;
	swapChainCreateInfo.minImageCount = desiredImages;
	swapChainCreateInfo.imageFormat = m_colorFormat;
	swapChainCreateInfo.imageColorSpace = m_colorSpace;
	swapChainCreateInfo.imageExtent = { swapChainExtent.width, swapChainExtent.height };

	// This is literally the same as GL_COLOR_ATTACHMENT0
	swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapChainCreateInfo.preTransform = (VkSurfaceTransformFlagBitsKHR)preTransform;
	swapChainCreateInfo.imageArrayLayers = 1; // Only one attachment
	swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // No sharing
	swapChainCreateInfo.queueFamilyIndexCount = 0;
	swapChainCreateInfo.pQueueFamilyIndices = NULL;
	swapChainCreateInfo.presentMode = presentMode;
	swapChainCreateInfo.clipped = true; // If we want clipping outside the extents

	// Alpha on the window surface should be opaque:
	// If it was not we could create transparent regions of our window which
	// would require support from the Window compositor
	swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	res = fpCreateSwapchainKHR(vulkan->getDevice(), &swapChainCreateInfo, NULL, &m_swapChain);

	if (res != VK_SUCCESS)
	{
		engine->showMessageErrorFatal("Fatal Vulkan Error", UString::format("Couldn't fpCreateSwapchainKHR(), returned %i", res));
		return;
	}

	// Now get the presentable images from the swap chain
	uint32_t imageCount;
	res = fpGetSwapchainImagesKHR(vulkan->getDevice(), m_swapChain, &imageCount, NULL);

	if (res != VK_SUCCESS)
	{
		engine->showMessageErrorFatal("Fatal Vulkan Error", UString::format("Couldn't fpGetSwapchainImagesKHR(NULL), returned %i", res));
		return;
	}

	m_images.resize(imageCount);

	res = fpGetSwapchainImagesKHR(vulkan->getDevice(), m_swapChain, &imageCount, &m_images[0]);
	if (res != VK_SUCCESS)
	{
		engine->showMessageErrorFatal("Fatal Vulkan Error", UString::format("Couldn't fpGetSwapchainImagesKHR(), returned %i", res));
		return;
	}

    // Create the image views for the swap chain. They will all be single
    // layer, 2D images, with no mipmaps.
    // Check the VkImageViewCreateInfo structure to see other views you
    // can potentially create.
	/*
    m_buffers.resize(imageCount);

	for (uint32_t i=0; i<imageCount; i++)
	{
		VkImageViewCreateInfo colorAttachmentView = { };
		colorAttachmentView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		colorAttachmentView.pNext = NULL;
		colorAttachmentView.format = m_colorFormat;
		colorAttachmentView.components =
		{
				VK_COMPONENT_SWIZZLE_R,
				VK_COMPONENT_SWIZZLE_G,
				VK_COMPONENT_SWIZZLE_B,
				VK_COMPONENT_SWIZZLE_A
		};
		colorAttachmentView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		colorAttachmentView.subresourceRange.baseMipLevel = 0;
		colorAttachmentView.subresourceRange.levelCount = 1;
		colorAttachmentView.subresourceRange.baseArrayLayer = 0;
		colorAttachmentView.subresourceRange.layerCount = 1;
		colorAttachmentView.viewType = VK_IMAGE_VIEW_TYPE_2D;
		colorAttachmentView.flags = 0; // mandatory

		// Wire them up
		m_buffers[i].image = m_images[i];
		// Transform images from the initial (undefined) layer to present layout
		// HACKHACK: TODO:
		////////setImageLayout(commandBuffer, m_buffers[i].image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		colorAttachmentView.image = m_buffers[i].image;
		// Create the view
		res = vkCreateImageView(vulkan->getDevice(), &colorAttachmentView, NULL, &m_buffers[i].view);
		if (res != VK_SUCCESS)
		{
			engine->showMessageErrorFatal("Fatal Vulkan Error", UString::format("Couldn't vkCreateImageView(), returned %i", res));
			return;
		}
	}
	*/

#endif
}

VulkanGraphicsInterface::~VulkanGraphicsInterface()
{
#ifdef MCENGINE_FEATURE_VULKAN

	for (auto &it : m_buffers)
	{
		vkDestroyImageView(vulkan->getDevice(), it.view, NULL);
	}

	fpDestroySwapchainKHR(vulkan->getDevice(), m_swapChain, NULL);
	vkDestroySurfaceKHR(vulkan->getInstance(), m_surface, NULL);

#endif
}

void VulkanGraphicsInterface::beginScene()
{
}

void VulkanGraphicsInterface::endScene()
{
}

void VulkanGraphicsInterface::clearDepthBuffer()
{
}

void VulkanGraphicsInterface::setColor(Color color)
{
}

void VulkanGraphicsInterface::setAlpha(float alpha)
{
}

void VulkanGraphicsInterface::drawPixels(int x, int y, int width, int height, Graphics::DRAWPIXELS_TYPE type, const void *pixels)
{
}

void VulkanGraphicsInterface::drawPixel(int x, int y)
{
}

void VulkanGraphicsInterface::drawLine(int x1, int y1, int x2, int y2)
{
}

void VulkanGraphicsInterface::drawLine(Vector2 pos1, Vector2 pos2)
{
}

void VulkanGraphicsInterface::drawRect(int x, int y, int width, int height)
{
}

void VulkanGraphicsInterface::drawRect(int x, int y, int width, int height, Color top, Color right, Color bottom, Color left)
{
}

void VulkanGraphicsInterface::fillRect(int x, int y, int width, int height)
{
}

void VulkanGraphicsInterface::fillRoundedRect(int x, int y, int width, int height, int radius)
{
}

void VulkanGraphicsInterface::fillGradient(int x, int y, int width, int height, Color topLeftColor, Color topRightColor, Color bottomLeftColor, Color bottomRightColor)
{
}

void VulkanGraphicsInterface::drawQuad(int x, int y, int width, int height)
{
}

void VulkanGraphicsInterface::drawQuad(Vector2 topLeft, Vector2 topRight, Vector2 bottomRight, Vector2 bottomLeft, Color topLeftColor, Color topRightColor, Color bottomRightColor, Color bottomLeftColor)
{
}

void VulkanGraphicsInterface::drawImage(Image *image)
{
}

void VulkanGraphicsInterface::drawString(McFont *font, UString text)
{
}

void VulkanGraphicsInterface::drawVAO(VertexArrayObject *vao)
{
}

void VulkanGraphicsInterface::drawVB(VertexBuffer *vb)
{
}

void VulkanGraphicsInterface::setClipRect(Rect clipRect)
{
}

void VulkanGraphicsInterface::pushClipRect(Rect clipRect)
{
}

void VulkanGraphicsInterface::popClipRect()
{
}

void VulkanGraphicsInterface::pushStencil()
{
}

void VulkanGraphicsInterface::fillStencil(bool inside)
{
}

void VulkanGraphicsInterface::popStencil()
{
}

void VulkanGraphicsInterface::setClipping(bool enabled)
{
}

void VulkanGraphicsInterface::setBlending(bool enabled)
{
}

void VulkanGraphicsInterface::setDepthBuffer(bool enabled)
{
}

void VulkanGraphicsInterface::setCulling(bool culling)
{
}

void VulkanGraphicsInterface::setAntialiasing(bool aa)
{
}

void VulkanGraphicsInterface::setWireframe(bool enabled)
{
}

void VulkanGraphicsInterface::flush()
{
}

std::vector<unsigned char> VulkanGraphicsInterface::getScreenshot()
{
	return std::vector<unsigned char>();
}

UString VulkanGraphicsInterface::getVendor()
{
	return "<VENDOR>";
}

UString VulkanGraphicsInterface::getModel()
{
	return "<MODEL>";
}

UString VulkanGraphicsInterface::getVersion()
{
	return "<VERSION>";
}

int VulkanGraphicsInterface::getVRAMTotal()
{
	return -1;
}

int VulkanGraphicsInterface::getVRAMRemaining()
{
	return -1;
}

void VulkanGraphicsInterface::onResolutionChange(Vector2 newResolution)
{
	m_vResolution = newResolution;
}

Image *VulkanGraphicsInterface::createImage(UString filePath, bool mipmapped)
{
	return NULL;
}

Image *VulkanGraphicsInterface::createImage(int width, int height, bool clampToEdge)
{
	return NULL;
}

RenderTarget *VulkanGraphicsInterface::createRenderTarget(int x, int y, int width, int height, Graphics::MULTISAMPLE_TYPE multiSampleType)
{
	return NULL;
}

Shader *VulkanGraphicsInterface::createShaderFromFile(UString vertexShaderFilePath, UString fragmentShaderFilePath)
{
	return NULL;
}

Shader *VulkanGraphicsInterface::createShaderFromSource(UString vertexShader, UString fragmentShader)
{
	return NULL;
}

#ifdef MCENGINE_FEATURE_VULKAN

void *VulkanGraphicsInterface::swapChainGetInstanceProc(const char *name)
{
	void *instanceProcAddr = vkGetInstanceProcAddr(vulkan->getInstance(), name);
	if (instanceProcAddr == NULL)
		engine->showMessageErrorFatal("Fatal Vulkan Error", UString::format("Couldn't get instance function pointer to %s!", name));

	return instanceProcAddr;
}

void *VulkanGraphicsInterface::swapChainGetDeviceProc(const char *name)
{
	void *deviceProcAddr = vkGetDeviceProcAddr(vulkan->getDevice(), name);
	if (deviceProcAddr == NULL)
		engine->showMessageErrorFatal("Fatal Vulkan Error", UString::format("Couldn't get device function pointer to %s!", name));

	return deviceProcAddr;
}

void VulkanGraphicsInterface::setImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout)
{
	// (c) SaschaWillems on Github

	VkImageMemoryBarrier imageMemoryBarrier = createImageMemoryBarrier();
	imageMemoryBarrier.oldLayout = oldImageLayout;
	imageMemoryBarrier.newLayout = newImageLayout;
	imageMemoryBarrier.image = image;
	imageMemoryBarrier.subresourceRange.aspectMask = aspectMask;
	imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
	imageMemoryBarrier.subresourceRange.levelCount = 1;
	imageMemoryBarrier.subresourceRange.layerCount = 1;

	// Undefined layout:
	//   Note: Only allowed as initial layout!
	//   Note: Make sure any writes to the image have been finished
	if (oldImageLayout == VK_IMAGE_LAYOUT_UNDEFINED)
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;

	// Old layout is color attachment:
	//   Note: Make sure any writes to the color buffer have been finished
	if (oldImageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	// Old layout is transfer source:
	//   Note: Make sure any reads from the image have been finished
	if (oldImageLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

	// Old layout is shader read (sampler, input attachment):
	//   Note: Make sure any shader reads from the image have been finished
	if (oldImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;

	// New layout is transfer destination (copy, blit):
	//   Note: Make sure any copyies to the image have been finished
	if (newImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

	// New layout is transfer source (copy, blit):
	//   Note: Make sure any reads from and writes to the image have been finished
	if (newImageLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
		imageMemoryBarrier.srcAccessMask = imageMemoryBarrier.srcAccessMask | VK_ACCESS_TRANSFER_READ_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	}

	// New layout is color attachment:
	//   Note: Make sure any writes to the color buffer hav been finished
	if (newImageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	}

	// New layout is depth attachment:
	//   Note: Make sure any writes to depth/stencil buffer have been finished
	if (newImageLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	// New layout is shader read (sampler, input attachment):
	//   Note: Make sure any writes to the image have been finished
	if (newImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	}

	// Put barrier inside the setup command buffer
	vkCmdPipelineBarrier(commandBuffer,
						 // Put the barriers for source and destination on
						 // top of the command buffer
						 VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
						 VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
						 0,
						 0, NULL,
						 0, NULL,
						 1, &imageMemoryBarrier);
}

VkImageMemoryBarrier VulkanGraphicsInterface::createImageMemoryBarrier()
{
	// (c) SaschaWillems on Github

	VkImageMemoryBarrier imageMemoryBarrier = {};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.pNext = NULL;

	// Some default values
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	return imageMemoryBarrier;
}

void VulkanGraphicsInterface::swapBuffers(VkQueue queue, VkSemaphore presentCompleteSemaphore, unsigned int *currentBuffer)
{
	fpAcquireNextImageKHR(vulkan->getDevice(), m_swapChain, 18446744073709551615ULL, presentCompleteSemaphore, (VkFence)0, currentBuffer);

	VkPresentInfoKHR presentInfo = { };
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = NULL;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &m_swapChain;
	presentInfo.pImageIndices = currentBuffer;
	fpQueuePresentKHR(queue, &presentInfo);
}

#endif
