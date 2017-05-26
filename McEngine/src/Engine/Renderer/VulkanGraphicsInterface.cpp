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

#include "NullImage.h"
#include "NullRenderTarget.h"
#include "NullShader.h"

VulkanGraphicsInterface::VulkanGraphicsInterface()
{
	if (!vulkan->isReady())
	{
		engine->showMessageErrorFatal("Fatal Vulkan Error", "Couldn't initialize VulkanGraphicsInterface because VulkanInterface is not ready!");
		exit(0);
	}

#ifdef MCENGINE_FEATURE_VULKAN

	// Vulkan has started up successfully, now try to get a surface
	VkResult res = VK_RESULT_MAX_ENUM;

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

#endif
}

void VulkanGraphicsInterface::init()
{

}

VulkanGraphicsInterface::~VulkanGraphicsInterface()
{
#ifdef MCENGINE_FEATURE_VULKAN

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

Image *VulkanGraphicsInterface::createImage(UString filePath, bool mipmapped)
{
	return new NullImage(filePath, mipmapped);
}

Image *VulkanGraphicsInterface::createImage(int width, int height, bool mipmapped)
{
	return new NullImage(width, height, mipmapped);
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

void VulkanGraphicsInterface::onTransformUpdate(Matrix4 &projectionMatrix, Matrix4 &worldMatrix)
{

}

#ifdef MCENGINE_FEATURE_VULKAN



#endif
