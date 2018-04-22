//================ Copyright (c) 2016, PG, All rights reserved. =================//
//
// Purpose:		vulkan wrapper
//
// $NoKeywords: $vk
//===============================================================================//

#include "VulkanInterface.h"
#include "VulkanGraphicsInterface.h"
#include "Engine.h"
#include "ConVar.h"

#include <string.h>

//ConVar vulkan_debug("vulkan_debug", false);

VulkanInterface *vulkan = NULL;

VulkanInterface::VulkanInterface()
{
	vulkan = this;
	m_bReady = false;
	m_iQueueFamilyIndex = 0;

#ifdef MCENGINE_FEATURE_VULKAN

	// instance extensions
	std::vector<const char *> enabledInstanceExtentions;
	enabledInstanceExtentions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

	// TODO: can't use WinVulkanInterface without adding Environment::createVulkan()
	// can't use WinVulkanGraphicsInterface, since VulkanInterface might be used without the VulkanGraphicsInterface
	// can't use env->getOS(), since the extension defines are per-OS in the header file (undefined otherwise)
	//Graphics *env->createRenderer();
	//enabledInstanceExtentions.insert(enabledInstanceExtentions.end(), b.begin(), b.end());

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)

	enabledInstanceExtentions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);

#else

	#error "TODO: add correct extension here"
	enabledInstanceExtentions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);

#endif

	// instance layers
	std::vector<const char *> enabledInstanceLayers;
	/*
	if (vulkan_debug.getBool())
		enabledInstanceLayers.push_back("VK_LAYER_LUNARG_standard_validation");
	*/

	// device layers
	std::vector<const char *> enabledDeviceLayers;

	// device extensions
	std::vector<const char *> enabledDeviceExtensions;
	enabledDeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	// device features
	VkPhysicalDeviceFeatures enabledDeviceFeatures = {};

	// list all available layers
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, NULL);
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
	debugLog("Vulkan: Found %i available layers:\n", layerCount);
	for (int i=0; i<availableLayers.size(); i++)
	{
		debugLog("Vulkan: Layer %i = %s v%i (%s)\n", i, availableLayers[i].description, availableLayers[i].implementationVersion, availableLayers[i].layerName);
	}

	// TODO: implement & handle debug layers and callbacks

	// application settings
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = NULL;
	appInfo.pApplicationName = "McEngine";
	appInfo.pEngineName = "McEngine";
	appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 4);

	// instance settings
	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pNext = NULL;
	createInfo.flags = 0;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = enabledInstanceExtentions.size();
	createInfo.ppEnabledExtensionNames = enabledInstanceExtentions.size() > 0 ? enabledInstanceExtentions.data() : NULL;
	createInfo.enabledLayerCount = enabledInstanceLayers.size();
	createInfo.ppEnabledLayerNames = enabledInstanceLayers.size() > 0 ? enabledInstanceLayers.data() : NULL;

	// create instance
	VkResult res = vkCreateInstance(&createInfo, NULL, &m_instance);
	if (res != VK_SUCCESS)
	{
		if (res == VK_ERROR_EXTENSION_NOT_PRESENT)
			engine->showMessageError("Vulkan Error", UString::format("Couldn't vkCreateInstance(), returned %d. Required Vulkan extensions are not present!", res));
		else if (res == VK_ERROR_LAYER_NOT_PRESENT)
			engine->showMessageError("Vulkan Error", UString::format("Couldn't vkCreateInstance(), returned %d. Required Vulkan layers are not present!", res));
		else
			engine->showMessageError("Vulkan Error", UString::format("Couldn't vkCreateInstance(), returned %d", res));
		return;
	}

	// iterate over all available devices
	unsigned int numDevices = 0;
	res = vkEnumeratePhysicalDevices(m_instance, &numDevices, NULL);
	if (res != VK_SUCCESS)
	{
		engine->showMessageError("Vulkan Error", UString::format("Couldn't vkEnumeratePhysicalDevices(), returned %d", res));
		return;
	}
	if (numDevices < 1)
	{
		engine->showMessageError("Vulkan Error", "Couldn't detect any Vulkan compatible devices!");
		return;
	}

	debugLog("Vulkan: Found %i compatible device(s)\n", numDevices);
	std::vector<VkPhysicalDevice> devices(numDevices);
	res = vkEnumeratePhysicalDevices(m_instance, &numDevices, &devices[0]);
	if (res != VK_SUCCESS)
	{
		engine->showMessageError("Vulkan Error", UString::format("Couldn't vkEnumeratePhysicalDevices(), returned %d", res));
		return;
	}

	// list all devices
	VkPhysicalDeviceProperties deviceProperties;
	for (unsigned int i=0; i<numDevices; i++)
	{
		memset(&deviceProperties, 0, sizeof(deviceProperties));
		vkGetPhysicalDeviceProperties(devices[i], &deviceProperties);
		debugLog("Vulkan Device #%i: Driver Version: %d\n", i, deviceProperties.driverVersion);
		debugLog("Vulkan Device #%i: Device Name:    %s\n", i, deviceProperties.deviceName);
		debugLog("Vulkan Device #%i: Device Type:    %d\n", i, deviceProperties.deviceType);
		debugLog("Vulkan Device #%i: API Version:    %d.%d.%d\n", i,
			(uint32_t)deviceProperties.apiVersion >> 22,
			((uint32_t)deviceProperties.apiVersion >> 12) & 0x3ff,
			(uint32_t)deviceProperties.apiVersion & 0xfff);
	}

	// and select a device
	int selectedDevice = 0;
	debugLog("Vulkan: Selecting device #%i.\n", selectedDevice);
	m_physicalDevice = devices[0];

	// get queue family properties
	std::vector<VkQueueFamilyProperties> queueFamilyProperties;
	uint32_t queueFamilyPropertyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyPropertyCount, NULL);
	if (queueFamilyPropertyCount < 1)
	{
		engine->showMessageError("Vulkan Error", UString::format("vkGetPhysicalDeviceQueueFamilyProperties() returned %i queueFamilyProperties", queueFamilyPropertyCount));
		return;
	}
	debugLog("Vulkan: %i available queue family properties\n", queueFamilyPropertyCount);
	queueFamilyProperties.resize(queueFamilyPropertyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyPropertyCount, queueFamilyProperties.data());

	// queues
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	const float defaultQueuePriority = 0.0f;
	{
		VkDeviceQueueCreateInfo deviceQueueCreateInfo = {};

		deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		deviceQueueCreateInfo.pNext = NULL;
		deviceQueueCreateInfo.flags = 0;

		// get index of first VK_QUEUE_GRAPHICS_BIT queue, default to 0 (first available queue)
		deviceQueueCreateInfo.queueFamilyIndex = 0;
		for (uint32_t i=0; i<queueFamilyProperties.size(); i++)
		{
			if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				debugLog("Vulkan: Selecting queue %i.\n", i);
				m_iQueueFamilyIndex = i;
				deviceQueueCreateInfo.queueFamilyIndex = i;
				break;
			}
		}
		deviceQueueCreateInfo.queueCount = 1;
		deviceQueueCreateInfo.pQueuePriorities = &defaultQueuePriority;

		queueCreateInfos.push_back(deviceQueueCreateInfo);
	}

	// create device
	VkDeviceCreateInfo deviceInfo = {};
	{
		// mandatory fields
		deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceInfo.pNext = NULL;
		deviceInfo.flags = 0;

		// layers + extensions + features
		deviceInfo.enabledLayerCount = enabledDeviceLayers.size();
		deviceInfo.ppEnabledLayerNames = enabledDeviceLayers.size() > 0 ? enabledDeviceLayers.data() : NULL;
		deviceInfo.enabledExtensionCount = enabledDeviceExtensions.size();
		deviceInfo.ppEnabledExtensionNames = enabledDeviceExtensions.size() > 0 ? enabledDeviceExtensions.data() : NULL;
		deviceInfo.pEnabledFeatures = &enabledDeviceFeatures;

		// queues
		deviceInfo.queueCreateInfoCount = queueCreateInfos.size();
		deviceInfo.pQueueCreateInfos = queueCreateInfos.size() > 0 ? queueCreateInfos.data() : NULL;
	}

	res = vkCreateDevice(m_physicalDevice, &deviceInfo, NULL, &m_device); // use default allocator
	if (res != VK_SUCCESS)
	{
		engine->showMessageError("Vulkan Error", UString::format("Couldn't vkCreateDevice(), returned %d", res));
		return;
	}

	// create command pool
	VkCommandPoolCreateInfo commandPoolInfo = {};
	commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.pNext = NULL;
	commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolInfo.queueFamilyIndex = m_iQueueFamilyIndex;

	res = vkCreateCommandPool(m_device, &commandPoolInfo, NULL, &m_commandPool); // use default allocator
	if (res != VK_SUCCESS)
	{
		engine->showMessageError("Vulkan Error", UString::format("Couldn't vkCreateCommandPool(), returned %d", res));
		return;
	}

	m_bReady = true;

#endif
}

VulkanInterface::~VulkanInterface()
{
#ifdef MCENGINE_FEATURE_VULKAN

	// (use default allocator)
	if (m_bReady)
	{
		vkDestroyCommandPool(m_device, m_commandPool, NULL);
		vkDestroyDevice(m_device, NULL);
		vkDestroyInstance(m_instance, NULL);
	}

#endif

	m_bReady = false;
	vulkan = NULL; // deref
}

void VulkanInterface::finish()
{
#ifdef MCENGINE_FEATURE_VULKAN

	vkDeviceWaitIdle(m_device);

#endif
}
