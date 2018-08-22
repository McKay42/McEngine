//================ Copyright (c) 2016, PG, All rights reserved. =================//
//
// Purpose:		vulkan implementation of Graphics
//
// $NoKeywords: $vkg
//===============================================================================//

#include "VulkanGraphicsInterface.h"
#include "Engine.h"

#ifdef MCENGINE_FEATURE_VULKAN

#include "WinEnvironment.h"
#include "LinuxEnvironment.h"

#include "NullImage.h"
#include "NullRenderTarget.h"
#include "NullShader.h"

#include "VertexArrayObject.h"


class VulkanSwapChain
{
public:
	VulkanSwapChain(VulkanGraphicsInterface *graphics)
	{
		m_graphics = graphics;
	}

	~VulkanSwapChain()
	{

	}

private:
	VulkanGraphicsInterface *m_graphics;
};



VulkanGraphicsInterface::VulkanGraphicsInterface() : NullGraphicsInterface()
{
	if (!vulkan->isReady())
	{
		engine->showMessageErrorFatal("Fatal Vulkan Error", "Couldn't initialize VulkanGraphicsInterface because VulkanInterface is not ready!");
		exit(0);
		return;
	}

	VkResult res = VK_RESULT_MAX_ENUM;

	// graphics queue
	vkGetDeviceQueue(vulkan->getDevice(), vulkan->getQueueFamilyIndex(), 0, &m_queue);



	// TODO: create swap chain etc.
	m_swapchain = new VulkanSwapChain(this);

	// get a surface to draw on
	m_surface = 0;

	// TODO: put all OS-specific code in WinVulkanInterface/LinuxVulkanInterface etc.
	// OS specific surface selection
#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)

	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo;
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.pNext = NULL;
	surfaceCreateInfo.flags = 0;
	surfaceCreateInfo.hinstance = ((WinEnvironment*)(env))->getHInstance();
	surfaceCreateInfo.hwnd = ((WinEnvironment*)(env))->getHwnd();
	res = vkCreateWin32SurfaceKHR(vulkan->getInstance(), &surfaceCreateInfo, NULL, &m_surface);

#else
	#error "TODO: add correct logic here"
#endif

	if (res != VK_SUCCESS)
	{
		engine->showMessageErrorFatal("Fatal Vulkan Error", UString::format("Couldn't vkCreate*SurfaceKHR(), returned %d", res));
		exit(0);
		return;
	}
}

void VulkanGraphicsInterface::init()
{

}

VulkanGraphicsInterface::~VulkanGraphicsInterface()
{
	if (m_surface != 0)
		vkDestroySurfaceKHR(vulkan->getInstance(), m_surface, NULL);

	SAFE_DELETE(m_swapchain);
}

void VulkanGraphicsInterface::beginScene()
{
}

void VulkanGraphicsInterface::endScene()
{
	// TODO: swapchain present

	vkQueueWaitIdle(m_queue);
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

void VulkanGraphicsInterface::setClipRect(McRect clipRect)
{
}

void VulkanGraphicsInterface::pushClipRect(McRect clipRect)
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

void VulkanGraphicsInterface::setVSync(bool enabled)
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

Image *VulkanGraphicsInterface::createImage(UString filePath, bool mipmapped, bool keepInSystemMemory)
{
	return new NullImage(filePath, mipmapped, keepInSystemMemory);
}

Image *VulkanGraphicsInterface::createImage(int width, int height, bool mipmapped, bool keepInSystemMemory)
{
	return new NullImage(width, height, mipmapped, keepInSystemMemory);
}

RenderTarget *VulkanGraphicsInterface::createRenderTarget(int x, int y, int width, int height, Graphics::MULTISAMPLE_TYPE multiSampleType)
{
	return new NullRenderTarget(x, y, width, height, multiSampleType);
}

Shader *VulkanGraphicsInterface::createShaderFromFile(UString vertexShaderFilePath, UString fragmentShaderFilePath)
{
	return new NullShader(vertexShaderFilePath, fragmentShaderFilePath, false);
}

Shader *VulkanGraphicsInterface::createShaderFromSource(UString vertexShader, UString fragmentShader)
{
	return new NullShader(vertexShader, fragmentShader, true);
}

VertexArrayObject *VulkanGraphicsInterface::createVertexArrayObject(Graphics::PRIMITIVE primitive, Graphics::USAGE_TYPE usage, bool keepInSystemMemory)
{
	return new VertexArrayObject(primitive, usage, keepInSystemMemory);
}

void VulkanGraphicsInterface::onTransformUpdate(Matrix4 &projectionMatrix, Matrix4 &worldMatrix)
{

}

#endif
