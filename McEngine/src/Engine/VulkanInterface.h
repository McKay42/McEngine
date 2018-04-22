//================ Copyright (c) 2016, PG, All rights reserved. =================//
//
// Purpose:		vulkan wrapper
//
// $NoKeywords: $vk
//===============================================================================//

#ifndef VULKANINTERFACE_H
#define VULKANINTERFACE_H

#include "cbase.h"

#ifdef MCENGINE_FEATURE_VULKAN

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)

#define VK_USE_PLATFORM_WIN32_KHR

#else

// #error "TODO: add correct define here"

#endif

#include <vulkan.h>

#endif

class VulkanInterface
{
public:
	VulkanInterface();
	~VulkanInterface();

	void finish();

	inline uint32_t getQueueFamilyIndex() const {return m_iQueueFamilyIndex;}
	inline bool isReady() const {return m_bReady;}

#ifdef MCENGINE_FEATURE_VULKAN

	// ILLEGAL:
	inline VkInstance getInstance() const {return m_instance;}
	inline VkPhysicalDevice getPhysicalDevice() const {return m_physicalDevice;}
	inline VkDevice getDevice() const {return m_device;}

#endif

private:

	bool m_bReady;
	uint32_t m_iQueueFamilyIndex;

#ifdef MCENGINE_FEATURE_VULKAN

	VkInstance m_instance;
	VkPhysicalDevice m_physicalDevice;
	VkDevice m_device;
	VkCommandPool m_commandPool;

#endif
};

extern VulkanInterface *vulkan;

#endif
