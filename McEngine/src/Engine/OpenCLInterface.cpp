//================ Copyright (c) 2013, PG, All rights reserved. =================//
//
// Purpose:		OpenCL 1.2 API wrapper
//
// $NoKeywords: $ocl
//===============================================================================//

#include "OpenCLInterface.h"

#include "ConVar.h"
#include "WinGLLegacyInterface.h"

#include <string.h>

#ifdef MCENGINE_FEATURE_OPENCL

#include "OpenGLHeaders.h"
#include "cl_gl.h"

#endif

OpenCLInterface *opencl = NULL;

OpenCLInterface::OpenCLInterface()
{
	opencl = this;

	m_iMaxMemAllocSizeInMB = 0;
	m_iGlobalMemSizeInMB = 0;

#ifdef MCENGINE_FEATURE_OPENCL

	m_context = NULL;
	m_commandQueue = NULL;

	// get all OpenCL devices
	cl_uint numPlatforms;
	cl_platform_id *platforms;

	const int numAttributes = 5;
	const cl_platform_info attributeTypes[numAttributes] = { CL_PLATFORM_NAME, CL_PLATFORM_VENDOR, CL_PLATFORM_VERSION, CL_PLATFORM_PROFILE, CL_PLATFORM_EXTENSIONS };

	// get platform count
	cl_int ret = clGetPlatformIDs(5, NULL, &numPlatforms);
	if (ret != CL_SUCCESS)
	{
		engine->showMessageError("OpenCL Error", UString::format("clGetPlatformIDs() returned %i", ret));
		return;
	}
	if (numPlatforms < 1)
	{
		engine->showMessageError("OpenCL Error", "clGetPlatformIDs() reported zero platforms!");
		return;
	}

	// get all platforms
	platforms = new cl_platform_id[numPlatforms];
	ret = clGetPlatformIDs(numPlatforms, platforms, NULL);
	if (ret != CL_SUCCESS)
	{
		engine->showMessageError("OpenCL Error", UString::format("clGetPlatformIDs() returned %i", ret));
		return;
	}

	// go through all platforms
	int bestDeviceIndex = 0;
	bool foundBestDevice = false;
	for (int i=0; i<numPlatforms; i++)
	{
		OPENCL_DEVICE dev;
		dev.name = "ERROR";
		dev.vendor = "ERROR";
		dev.version = "ERROR";
		dev.profile = "ERROR";
		dev.extensions = "ERROR";

		for (int j=0; j<numAttributes; j++)
		{
			// get string length
			size_t infoSize;
			ret = clGetPlatformInfo(platforms[i], attributeTypes[j], 0, NULL, &infoSize);
			if (ret != CL_SUCCESS)
				continue;

			// get string
			char *infoString = new char[infoSize];
			ret = clGetPlatformInfo(platforms[i], attributeTypes[j], infoSize, infoString, NULL);
			if (ret != CL_SUCCESS)
			{
				delete[] infoString;
				continue;
			}

			switch (j)
			{
			case 0:
				dev.name = UString(infoString);
				break;
			case 1:
				dev.vendor = UString(infoString);
				break;
			case 2:
				dev.version = UString(infoString);
				break;
			case 3:
				dev.profile = UString(infoString);
				break;
			case 4:
				dev.extensions = UString(infoString);
				break;
			}

			delete[] infoString;
		}

		// at the same time, determine the best device
		if (!foundBestDevice && dev.name.find("NVIDIA") != -1)
		{
			bestDeviceIndex = i;
			foundBestDevice = true;
		}

		// debug devices

		if (true)
		{
			debugLog("OpenCL Device #%i: %s\n", i, dev.name.toUtf8());
			debugLog("OpenCL Device #%i: %s\n", i, dev.vendor.toUtf8());
			debugLog("OpenCL Device #%i: %s\n", i, dev.version.toUtf8());
			debugLog("OpenCL Device #%i: %s\n", i, dev.profile.toUtf8());
			debugLog("OpenCL Device #%i: %s\n", i, dev.extensions.toUtf8());
		}


		m_devices.push_back(dev);
	}
	debugLog("OpenCL: Automatically selected \"%s\" as default device.\n", m_devices[bestDeviceIndex].name.toUtf8());
	debugLog("OpenCL: %s\n", m_devices[bestDeviceIndex].version.toUtf8());


	// get device ID
	m_deviceID = NULL;
	cl_uint num_devices = 0;

	ret = clGetDeviceIDs( platforms[bestDeviceIndex], CL_DEVICE_TYPE_DEFAULT, 1, &m_deviceID, &num_devices);
	if (ret != CL_SUCCESS)
	{
		engine->showMessageError("OpenCL Error", UString::format("clGetDeviceIDs() returned %i", ret));
		return;
	}
	if (num_devices < 1)
	{
		engine->showMessageError("OpenCL Error", "clGetDeviceIDs() reported zero devices!");
		return;
	}
	debugLog("OpenCL: num_devices = %i, num_platforms = %i\n", num_devices, numPlatforms);


	// create shared context
//#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)

	/*
	// HACKHACK: hardcoded WinGLInterface
	cl_context_properties akProperties[] =
	{
		CL_CONTEXT_PLATFORM, (cl_context_properties)platforms[bestDeviceIndex],
		CL_GL_CONTEXT_KHR, (cl_context_properties)((WinGLInterface*)engine->getGraphics())->getGLContext(),
		CL_WGL_HDC_KHR, (cl_context_properties)((WinGLInterface*)engine->getGraphics())->getGLHDC(),
		0
	};

	m_context = clCreateContextFromType( akProperties, CL_DEVICE_TYPE_DEFAULT, NULL, NULL, &ret);
	*/

//#else

	// create normal context
	m_context = clCreateContext( NULL, 1, &m_deviceID, NULL, NULL, &ret);

//#endif

	if (m_context == NULL)
	{
		engine->showMessageError("OpenCL Error", "clCreateContext() returned NULL!");
		return;
	}
	if (ret != CL_SUCCESS)
	{
		engine->showMessageError("OpenCL Error", UString::format("clCreateContext() returned %i\n",ret) );
		return;
	}


	// create command queue
	m_commandQueue = clCreateCommandQueue(m_context, m_deviceID, 0, &ret);
	if (m_commandQueue == NULL)
	{
		engine->showMessageError("OpenCL Error", "clCreateCommandQueue() returned NULL!");
		return;
	}
	if (ret != CL_SUCCESS)
	{
		engine->showMessageError("OpenCL Error", UString::format("clCreateCommandQueue() returned %i\n",ret) );
		return;
	}

	delete[] platforms;


	// get device parameters
	cl_ulong globalMemSize;
	clGetDeviceInfo(m_deviceID, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), &globalMemSize, NULL);
	cl_ulong maxMemAllocSize;
	clGetDeviceInfo(m_deviceID, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(cl_ulong), &maxMemAllocSize, NULL);

	m_iGlobalMemSizeInMB = globalMemSize/1024/1024;
	m_iMaxMemAllocSizeInMB = maxMemAllocSize/1024/1024;

	debugLog("OpenCL: CL_DEVICE_GLOBAL_MEM_SIZE = %i MB\n", m_iGlobalMemSizeInMB);
	debugLog("OpenCL: CL_DEVICE_MAX_MEM_ALLOC_SIZE = %i MB\n", m_iMaxMemAllocSizeInMB);

#endif
}

OpenCLInterface::~OpenCLInterface()
{
#ifdef MCENGINE_FEATURE_OPENCL

	cleanup();

	cl_int ret = CL_INVALID_VALUE;

	// after releasing all resources, release the queue
	if (m_commandQueue != NULL)
	{
		ret = clReleaseCommandQueue(m_commandQueue);
		debugLog("OpenCL: clReleaseCommandQueue() returned %i\n",ret);
		if (ret != CL_SUCCESS)
			engine->showMessageError("OpenCL Error", UString::format("clReleaseCommandQueue(m_commandQueue) returned %i", ret));
	}

	// finally, release the context
	ret = clReleaseContext(m_context);
	debugLog("OpenCL: clReleaseContext() returned %i\n",ret);
	if (ret != CL_SUCCESS)
		engine->showMessageError("OpenCL Error", UString::format("clReleaseContext() returned %i", ret));

#endif

	opencl = NULL;
}

void OpenCLInterface::cleanup()
{
#ifdef MCENGINE_FEATURE_OPENCL

	// first, flush and finish the queue
	cl_int ret = CL_INVALID_VALUE;
	if (m_commandQueue != NULL)
	{
		ret = clFlush(m_commandQueue);
		debugLog("OpenCL: clFlush(m_commandQueue) returned %i\n",ret);
		if (ret != CL_SUCCESS)
		{
			debugLog("which is %i\n",(int)(ret == CL_SUCCESS));
			engine->showMessageError("OpenCL Error", UString::format("clFlush(m_commandQueue) returned %i", ret));
		}
		ret = clFinish(m_commandQueue);
		debugLog("OpenCL: clFinish(m_commandQueue) returned %i\n",ret);
		if (ret != CL_SUCCESS)
			engine->showMessageError("OpenCL Error", UString::format("clFinish(m_commandQueue) returned %i", ret));
	}

	// release memory objects
	for (int i=0; i<m_vBuffers.size(); i++)
	{
		if (m_vBuffers[i] != NULL)
		{
			ret = clReleaseMemObject(m_vBuffers[i]);
			debugLog("OpenCL: clReleaseMemObject(m_vBuffers[%i]) returned %i\n", i, ret);
			if (ret != CL_SUCCESS)
				engine->showMessageError("OpenCL Error", UString::format("clReleaseMemObject(m_vBuffers[%i]) returned %i", i, ret));
		}
	}
	m_vBuffers.clear();
	m_vBuffers.swap(m_vBuffers);
	m_vBufferIndex.clear();
	m_vBufferIndex.swap(m_vBufferIndex);

	// release kernels
	for (int i=0; i<m_vKernels.size(); i++)
	{
		ret = clReleaseKernel(m_vKernels[i]);
		debugLog("OpenCL: clReleaseKernel(m_vKernels[%i]) returned %i!\n", i, ret);
		if (ret != CL_SUCCESS)
			engine->showMessageError("OpenCL Error", UString::format("clReleaseKernel(m_vKernels[%i]) returned %i", i, ret));
	}
	m_vKernels.clear();
	m_vKernels.swap(m_vKernels);

	// release programs
	for (int i=0; i<m_vPrograms.size(); i++)
	{
		ret = clReleaseProgram(m_vPrograms[i]);
		debugLog("OpenCL: clReleaseProgram(m_vPrograms[%i]) returned %i!\n", i, ret);
		if (ret != CL_SUCCESS)
			engine->showMessageError("OpenCL Error", UString::format("clReleaseProgram(m_vPrograms[%i]) returned %i", i, ret));
	}
	m_vPrograms.clear();
	m_vPrograms.swap(m_vPrograms);

#endif
}

void OpenCLInterface::releaseKernel(int kernel)
{
#ifdef MCENGINE_FEATURE_OPENCL

	if (kernel < 0 || kernel > (int)(m_vKernels.size()-1) || kernel > (int)(m_vPrograms.size()-1))
	{
		engine->showMessageError("OpenCL Error", UString::format("Invalid releaseKernel(%i)", kernel));
		return;
	}

	cl_int ret = CL_INVALID_VALUE;
	ret = clReleaseKernel(m_vKernels[kernel]);
	m_vKernels.erase(m_vKernels.begin() + kernel); // HACKHACK: multiple kernels will break the indices!
	debugLog("OpenCL: clReleaseKernel(m_vKernels[%i]) returned %i!\n", kernel, ret);
	if (ret != CL_SUCCESS)
		engine->showMessageError("OpenCL Error", UString::format("clReleaseKernel(m_vKernels[%i]) returned %i", kernel, ret));

	ret = clReleaseProgram(m_vPrograms[kernel]);
	m_vPrograms.erase(m_vPrograms.begin() + kernel); // HACKHACK: multiple kernels will break the indices!
	debugLog("OpenCL: clReleaseProgram(m_vKernels[%i]) returned %i!\n", kernel, ret);
	if (ret != CL_SUCCESS)
		engine->showMessageError("OpenCL Error", UString::format("clReleaseProgram(m_vKernels[%i]) returned %i", kernel, ret));

#endif
}

void OpenCLInterface::flush()
{
#ifdef MCENGINE_FEATURE_OPENCL

	clFlush(m_commandQueue);

#endif
}

void OpenCLInterface::finish()
{
#ifdef MCENGINE_FEATURE_OPENCL

	clFinish(m_commandQueue);

#endif
}

int OpenCLInterface::createKernel(UString kernelSourceCode, UString functionName)
{
#ifdef MCENGINE_FEATURE_OPENCL

	// create program from kernel source
	cl_int ret = CL_INVALID_VALUE;
	int kernelLength = kernelSourceCode.length();
	const char *kernelSource = kernelSourceCode.toUtf8();
	cl_program program = clCreateProgramWithSource(m_context, 1, (const char**)&kernelSource, (const size_t *)&kernelLength, &ret);

	if (program == NULL)
	{
		engine->showMessageError("OpenCL Error", "clCreateProgramWithSource() returned NULL!");
		return -1;
	}

	// push back for cleanup if it wasn't NULL
	m_vPrograms.push_back(program);

	if (ret != CL_SUCCESS)
	{
		engine->showMessageError("OpenCL Error", UString::format("clCreateProgramWithSource() returned %i!", ret));
		return -1;
	}

	// build program
	ret = clBuildProgram(program, 1, &m_deviceID, NULL, NULL, NULL);

	// log errors
	if (ret == CL_BUILD_PROGRAM_FAILURE)
	{
		debugLog("------------------OpenCL Compiler Error------------------\n");

	    size_t logSize;
	    clGetProgramBuildInfo(program, m_deviceID, CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);
	    char *log = new char[logSize+1];
	    clGetProgramBuildInfo(program, m_deviceID, CL_PROGRAM_BUILD_LOG, logSize, log, NULL);
	    log[logSize] = '\0';
	    debugLog("%s\n",log);
	    env->showMessageWarning("OpenCL Compiler Error", "Either a syntax error or an OpenCL compiler compatibility error, take a look at the console for details.");
	    delete[] log;

	    debugLog("--------------------------------------------------------\n");
	    return -1;
	}

	if (ret != CL_SUCCESS)
	{
		engine->showMessageError("OpenCL Error", UString::format("clBuildProgram() returned %i! (possible syntax error?)", ret));
		return -1;
	}

	// create kernel
	cl_kernel kernel = clCreateKernel(program, functionName.toUtf8(), &ret);

	if (kernel == NULL)
	{
		engine->showMessageError("OpenCL Error", UString::format("clCreateKernel() returned NULL! ret = %i", ret));
		return -1;
	}

	m_vKernels.push_back(kernel);

	if (ret != CL_SUCCESS)
	{
		engine->showMessageError("OpenCL Error", UString::format("clCreateKernel() returned %i!", ret));
		return -1;
	}

	return m_vKernels.size()-1;

#else
	return -1;
#endif
}

int OpenCLInterface::createKernel(const char *kernelSourceCode, UString functionName)
{
#ifdef MCENGINE_FEATURE_OPENCL

	// create program from kernel source
	cl_int ret = CL_INVALID_VALUE;
	int kernelLength = strlen(kernelSourceCode);
	const char *kernelSource = kernelSourceCode;
	cl_program program = clCreateProgramWithSource(m_context, 1, (const char**)&kernelSource, (const size_t *)&kernelLength, &ret);

	if (program == NULL)
	{
		engine->showMessageError("OpenCL Error", "clCreateProgramWithSource() returned NULL!");
		return -1;
	}

	// push back for cleanup if it wasn't NULL
	m_vPrograms.push_back(program);

	if (ret != CL_SUCCESS)
	{
		engine->showMessageError("OpenCL Error", UString::format("clCreateProgramWithSource() returned %i!", ret));
		return -1;
	}

	// build program
	ret = clBuildProgram(program, 1, &m_deviceID, NULL, NULL, NULL);

	// log errors
	if (ret == CL_BUILD_PROGRAM_FAILURE)
	{
		debugLog("------------------OpenCL Compiler Error------------------\n");

	    size_t logSize;
	    clGetProgramBuildInfo(program, m_deviceID, CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);
	    char *log = new char[logSize+1];
	    clGetProgramBuildInfo(program, m_deviceID, CL_PROGRAM_BUILD_LOG, logSize, log, NULL);
	    log[logSize] = '\0';
	    debugLog("%s\n",log);
	    env->showMessageWarning("OpenCL Compiler Error", "Probably a syntax error, take a look at the console for details");
	    delete[] log;

	    debugLog("--------------------------------------------------------\n");
	    return -1;
	}

	if (ret != CL_SUCCESS)
	{
		engine->showMessageError("OpenCL Error", UString::format("clBuildProgram() returned %i! (possible syntax error?)", ret));
		return -1;
	}

	// create kernel
	cl_kernel kernel = clCreateKernel(program, functionName.toUtf8(), &ret);

	if (kernel == NULL)
	{
		engine->showMessageError("OpenCL Error", UString::format("clCreateKernel() returned NULL! ret = %i", ret));
		return -1;
	}

	m_vKernels.push_back(kernel);

	if (ret != CL_SUCCESS)
	{
		engine->showMessageError("OpenCL Error", UString::format("clCreateKernel() returned %i!", ret));
		return -1;
	}

	return m_vKernels.size()-1;

#else
	return -1;
#endif
}

void OpenCLInterface::setKernelArg(int kernel, unsigned int argumentNumber, int buffer)
{
#ifdef MCENGINE_FEATURE_OPENCL

	if (kernel < 0 || kernel > (int)(m_vKernels.size()-1) || argumentNumber < 0 || buffer < 0 || buffer > (int)(m_vBuffers.size()-1))
	{
		engine->showMessageError("OpenCL Error", UString::format("Illegal arguments to setKernelArg(%i, %i, %i)!", kernel, argumentNumber, buffer));
		return;
	}

	cl_int ret = CL_INVALID_VALUE;
	ret = clSetKernelArg(m_vKernels[kernel], argumentNumber, sizeof(cl_mem), (void *)&(m_vBuffers[buffer]));

	if (ret != CL_SUCCESS)
		engine->showMessageError("OpenCL Error", UString::format("clSetKernelArg(%i, %i, %i) returned %i!", kernel, argumentNumber, buffer, ret));
#endif
}

void OpenCLInterface::executeKernel(int kernel, unsigned int numLoops, const size_t *globalItemSize, const size_t *localItemSize)
{
#ifdef MCENGINE_FEATURE_OPENCL

	if (kernel < 0 || kernel > (int)(m_vKernels.size()-1) || globalItemSize == NULL || localItemSize == NULL)
	{
		engine->showMessageError("OpenCL Error", UString::format("Illegal arguments to executeKernel(%i, %i, %i)!", kernel, globalItemSize, localItemSize));
		return;
	}

	cl_int ret = CL_INVALID_VALUE;
	ret = clEnqueueNDRangeKernel(m_commandQueue, m_vKernels[kernel], numLoops, NULL, globalItemSize, localItemSize, 0, NULL, NULL);

	if (ret != CL_SUCCESS)
	{
		if (ret == -1000)
			engine->showMessageError("OpenCL Error", UString::format("clEnqueueNDRangeKernel(%i, %i, %i) returned %i!\nYou probably forgot to opencl->aquireGLObject()!", kernel, globalItemSize, localItemSize, ret));
		else
			engine->showMessageError("OpenCL Error", UString::format("clEnqueueNDRangeKernel(%i, %i, %i) returned %i!\n\n(You can quit the engine by closing the console!)", kernel, globalItemSize, localItemSize, ret));
	}

#endif
}

void OpenCLInterface::aquireGLObject(int object)
{
#ifdef MCENGINE_FEATURE_OPENCL

	if (object < 0 || object > (int)(m_vBufferIndex.size()-1))
	{
		engine->showMessageError("OpenCL Error", UString::format("Illegal arguments to aquireGLObjects(%i)!", object));
		return;
	}

	if (m_vBufferIndex[object] > (int)(m_vBuffers.size()-1) || m_vBufferIndex[object] < 0)
	{
		engine->showMessageError("OpenCL Error", UString::format("Illegal arguments to aquireGLObject(%i)!", m_vBufferIndex[object]));
		return;
	}

	cl_int ret = CL_INVALID_VALUE;
	ret = clEnqueueAcquireGLObjects(m_commandQueue, 1,  &m_vBuffers[m_vBufferIndex[object]], 0, 0, NULL);

	if (ret != CL_SUCCESS)
		engine->showMessageError("OpenCL Error", UString::format("clEnqueueAcquireGLObjects(%i) returned %i!\n", object));

#endif
}

void OpenCLInterface::releaseGLObject(int object)
{
#ifdef MCENGINE_FEATURE_OPENCL

	if (object < 0 || object > (int)(m_vBufferIndex.size()-1))
	{
		engine->showMessageError("OpenCL Error", UString::format("Illegal arguments to releaseGLObject(%i)!", object));
		return;
	}

	if (m_vBufferIndex[object] > (int)(m_vBuffers.size()-1) || m_vBufferIndex[object] < 0)
	{
		engine->showMessageError("OpenCL Error", UString::format("Illegal arguments to releaseGLObject(%i)!", m_vBufferIndex[object]));
		return;
	}

	cl_int ret = CL_INVALID_VALUE;
	ret = clEnqueueReleaseGLObjects(m_commandQueue, 1,  &m_vBuffers[m_vBufferIndex[object]], 0, 0, NULL);

	if (ret != CL_SUCCESS)
		engine->showMessageError("OpenCL Error", UString::format("clEnqueueReleaseGLObjects(%i) returned %i!\n", object));

#endif
}

int OpenCLInterface::createTexture(unsigned int sourceImage, bool readable, bool writeable)
{
#ifdef MCENGINE_FEATURE_OPENCL

	if (!sourceImage)
	{
		engine->showMessageError("OpenCL Error", "createTexture() invalid sourceImage!");
		return -1;
	}
	if (!readable && !writeable)
	{
		engine->showMessageError("OpenCL Error", "createTexture() !readable && !writeable, impossible");
		return -1;
	}

	cl_int error;
	cl_mem img = clCreateFromGLTexture2D(m_context, (readable && writeable) ? CL_MEM_READ_WRITE : ( (readable && !writeable) ? CL_MEM_READ_ONLY : CL_MEM_WRITE_ONLY ), GL_TEXTURE_2D, 0, sourceImage, &error);

	if (!img || error)
	{
		engine->showMessageError("OpenCL Error", UString::format("clCreateFromGLTexture2D() returned %i, error = %i\n", img, error));
		return -1;
	}

	m_vBuffers.push_back(img);
	m_vBufferIndex.push_back(m_vBuffers.size()-1);
	return m_vBufferIndex.size()-1;

#else
	return -1;
#endif
}

int OpenCLInterface::createTexture3D(unsigned int sourceImage)
{
#ifdef MCENGINE_FEATURE_OPENCL

	if (!sourceImage)
	{
		engine->showMessageError("OpenCL Error", "createTexture3D() invalid sourceImage!");
		return -1;
	}

	cl_int error;
	cl_mem img = clCreateFromGLTexture3D(m_context, CL_MEM_READ_ONLY, GL_TEXTURE_3D, 0, sourceImage, &error);

	if (!img || error)
	{
		engine->showMessageError("OpenCL Error", UString::format("clCreateFromGLTexture3D(%i) returned %i, error = %i\n", sourceImage, img, error));
		return -1;
	}

	m_vBuffers.push_back(img);
	m_vBufferIndex.push_back(m_vBuffers.size()-1);
	return m_vBufferIndex.size()-1;

#else
	return -1;
#endif
}

int OpenCLInterface::getWorkGroupSize(int kernel)
{
#ifdef MCENGINE_FEATURE_OPENCL

	if (kernel < 0 || kernel > (int)(m_vKernels.size()-1))
		return -1;

	size_t size;
	int ret = clGetKernelWorkGroupInfo(m_vKernels[kernel], m_deviceID, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), (void*)&size, NULL);

	if (ret != CL_SUCCESS)
		engine->showMessageError("OpenCL Error", UString::format("clGetKernelWorkGroupInfo() returned %i\n", ret));

	return size;

#else
	return -1;
#endif
}

void OpenCLInterface::releaseBuffer(int buffer)
{
#ifdef MCENGINE_FEATURE_OPENCL

	if (buffer > -1 || buffer < m_vBufferIndex.size())
	{
		if (m_vBufferIndex[buffer] > m_vBuffers.size() || m_vBufferIndex[buffer] < 0)
		{
			engine->showMessageError("OpenCL Error", UString::format("Invalid Buffer for releaseBuffer(%i)!\n", m_vBufferIndex[buffer]));
			return;
		}

		cl_int ret = clReleaseMemObject(m_vBuffers[m_vBufferIndex[buffer]]);
		m_vBuffers[m_vBufferIndex[buffer]] = NULL; //HACKHACK: can't erase() because then the indices would change, breaking all existing buffers!
		debugLog("OpenCL: clReleaseMemObject(m_vBuffers[%i]) returned %i!\n", buffer, ret);
		if (ret != CL_SUCCESS)
			engine->showMessageError("OpenCL Error", UString::format("clReleaseMemObject(m_vBuffers[%i]) returned %i", buffer, ret));
	}

#endif
}


//**********************//
//	OpenCL ConCommands	//
//**********************//

void _opencl_listdevices( void )
{
	debugLog("OpenCL Devices:\n");

	std::vector<OpenCLInterface::OPENCL_DEVICE> devices = opencl->getDevices();
	for (int i=0; i<devices.size(); i++)
	{
		debugLog(" - Platform %i:\n", i);
		debugLog("      Name = %s\n", devices[i].name.toUtf8());
		debugLog("      Vendor = %s\n", devices[i].vendor.toUtf8());
		debugLog("      Version = %s\n", devices[i].version.toUtf8());
		debugLog("      Profile = %s\n\n", devices[i].profile.toUtf8());
		//debugLog("      Extensions = %s\n\n", devices[i].extensions.toUtf8());
	}
}

ConVar _opencl_listdevices_("opencl_listdevices", _opencl_listdevices);
