//================ Copyright (c) 2016, PG, All rights reserved. =================//
//
// Purpose:		vulkan implementation of Graphics
//
// $NoKeywords: $vkg
//===============================================================================//

#ifndef VULKANGRAPHICSINTERFACE_H
#define VULKANGRAPHICSINTERFACE_H

#include "VulkanInterface.h"
#include "Graphics.h"
#include "cbase.h"

// TODO: implement VulkanGraphicsInterface

class VulkanGraphicsInterface : public Graphics
{
public:
	VulkanGraphicsInterface();
	virtual ~VulkanGraphicsInterface();

	// scene
	void beginScene();
	void endScene();

	// color
	void setColor(Color color);
	void setAlpha(float alpha);

	// drawing
	void drawPixel(int x, int y);
	void drawLine(int x1, int y1, int x2, int y2);
	void drawLine(Vector2 pos1, Vector2 pos2);
	void drawRect(int x, int y, int width, int height);
	void drawRect(int x, int y, int width, int height, Color top, Color right, Color bottom, Color left);

	void fillRect(int x, int y, int width, int height);
	void fillRoundedRect(int x, int y, int width, int height, int radius);
	void fillGradient(int x, int y, int width, int height, Color topLeftColor, Color topRightColor, Color bottomLeftColor, Color bottomRightColor);

	void drawQuad(int x, int y, int width, int height);
	void drawQuad(Vector2 topLeft, Vector2 topRight, Vector2 bottomRight, Vector2 bottomLeft, Color topLeftColor, Color topRightColor, Color bottomRightColor, Color bottomLeftColor);

	void drawImage(Image *image);

	void drawString(McFont *font, UString text);

	// transforms
	void translate(int x, int y);
	void rotate(float deg);
	void scale(float x, float y);

	void pushTransform();
	void popTransform();

	// clipping
	void setClipRect(Rect clipRect);
	void pushClipRect(Rect clipRect);
	void popClipRect();

	// stencil
	void pushStencil();
	void fillStencil(bool inside);
	void popStencil();

	// 3d gui scenes
	void push3DScene(Rect region);
	void pop3DScene();
	void translate3DScene(float x, float y, float z);
	void rotate3DScene(float rotx, float roty, float rotz);
	void offset3DScene(float x, float y, float z);

	// device settings
	void setCulling(bool culling);
	void setVSync(bool vsync);
	void setAntialiasing(bool aa);

	// renderer info
	UString getVendor();
	UString getModel();
	UString getVersion();
	int getVRAMTotal();
	int getVRAMRemaining();

	void onResolutionChange();

private:

#ifdef MCENGINE_FEATURE_VULKAN

	void *swapChainGetInstanceProc(const char *name);
	void *swapChainGetDeviceProc(const char *name);
	void setImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout);
	VkImageMemoryBarrier createImageMemoryBarrier();
	void swapBuffers(VkQueue queue, VkSemaphore presentCompleteSemaphore, unsigned int *currentBuffer);

	// swap chain stuff
	VkSurfaceKHR m_surface;
	VkFormat m_colorFormat;
	VkColorSpaceKHR m_colorSpace;

	VkSwapchainKHR m_swapChain;
	std::vector<VkImage> m_images;

	struct SwapChainBuffer
	{
		VkImage image;
		VkImageView view;
	};
	std::vector<SwapChainBuffer> m_buffers;
	size_t m_nodeIndex;

	PFN_vkGetPhysicalDeviceSurfaceSupportKHR fpGetPhysicalDeviceSurfaceSupportKHR;
	PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR fpGetPhysicalDeviceSurfaceCapabilitiesKHR;
	PFN_vkGetPhysicalDeviceSurfaceFormatsKHR fpGetPhysicalDeviceSurfaceFormatsKHR;
	PFN_vkGetPhysicalDeviceSurfacePresentModesKHR fpGetPhysicalDeviceSurfacePresentModesKHR;
	PFN_vkCreateSwapchainKHR fpCreateSwapchainKHR;
	PFN_vkDestroySwapchainKHR fpDestroySwapchainKHR;
	PFN_vkGetSwapchainImagesKHR fpGetSwapchainImagesKHR;
	PFN_vkAcquireNextImageKHR fpAcquireNextImageKHR;
	PFN_vkQueuePresentKHR fpQueuePresentKHR;

#endif
};

#endif
