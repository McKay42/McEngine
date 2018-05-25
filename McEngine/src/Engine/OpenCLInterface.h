//================ Copyright (c) 2013, PG, All rights reserved. =================//
//
// Purpose:		OpenCL 1.2 API wrapper
//
// $NoKeywords: $ocl
//===============================================================================//

#ifndef OPENCLINTERFACE_H
#define OPENCLINTERFACE_H

#include "Engine.h"

#ifdef MCENGINE_FEATURE_OPENCL

#include "cl.h"

#endif

class OpenCLInterface
{
public:
	OpenCLInterface();
	~OpenCLInterface();

	// buffers
	template <typename T>
	int createBuffer(unsigned int numberOfObjects = 1, bool readable = true, bool writeable = true);
	template <typename T>
	void writeBuffer(int buffer, unsigned int numberOfObjects, const void *ptr);
	template <typename T>
	void writeBuffer(int buffer, unsigned int numberOfObjects, unsigned int startObjectIndex, const void *ptr);
	template <typename T>
	void readBuffer(int buffer, unsigned int numberOfObjects, void *ptr);
	void releaseBuffer(int buffer);
	template <typename T>
	void updateBuffer(int buffer, unsigned int numberOfObjects, bool readable, bool writeable);

	// OpenGL interop
	void aquireGLObject(int object);
	void releaseGLObject(int object);
	int createTexture(unsigned int sourceImage, bool readable = true, bool writeable = false);
	int createTexture3D(unsigned int sourceImage); // currently OpenCL 1.1. only supports native reads from 3d images

	// workgroup size
	int getWorkGroupSize(int kernel);

	// kernel
	int createKernel(UString kernelSourceCode, UString functionName);
	int createKernel(const char *kernelSourceCode, UString functionName);
	void setKernelArg(int kernel, unsigned int argumentNumber, int buffer);
	template <typename T>
	void setKernelArg(int kernel, unsigned int argumentNumber, T argument);

	void executeKernel(int kernel, unsigned int numLoops, const size_t *globalItemSize, const size_t *localItemSize);

	// misc
	void cleanup();
	void releaseKernel(int kernel);
	void flush();
	void finish();

	inline int getMaxMemAllocSizeInMB() {return m_iMaxMemAllocSizeInMB;}
	inline int getGlobalMemSizeInMB() {return m_iGlobalMemSizeInMB;}


	struct OPENCL_DEVICE
	{
		UString name;
		UString vendor;
		UString version;
		UString profile;
		UString extensions;
	};

	std::vector<OPENCL_DEVICE> getDevices() {return m_devices;}

private:
	std::vector<OPENCL_DEVICE> m_devices;

	unsigned int m_iMaxMemAllocSizeInMB;
	unsigned int m_iGlobalMemSizeInMB;

#ifdef MCENGINE_FEATURE_OPENCL

	cl_device_id m_deviceID;
	cl_context m_context;
	cl_command_queue m_commandQueue;

	std::vector<cl_mem> m_vBuffers;
	std::vector<int> m_vBufferIndex;
	std::vector<cl_program> m_vPrograms;
	std::vector<cl_kernel> m_vKernels;

#endif
};

extern OpenCLInterface *opencl;



//**************************//
//	template function defs  //
//**************************//

template <typename T>
int OpenCLInterface::createBuffer(unsigned int numberOfObjects, bool readable, bool writeable)
{
#ifdef MCENGINE_FEATURE_OPENCL

	if (numberOfObjects < 1)
	{
		engine->showMessageError("OpenCL Error", UString::format("Can't createBuffer() with %i numberOfObjects you idiot!\n", numberOfObjects) );
		return -1;
	}

	unsigned int requestedMemSizeInMB = (numberOfObjects * sizeof(T))/1024/1024;
	if (requestedMemSizeInMB > m_iMaxMemAllocSizeInMB)
	{
		debugLog("OpenCL Warning: Trying to allocate %i MB for a single buffer, but the maximum supported size is %i MB!\n", requestedMemSizeInMB, m_iMaxMemAllocSizeInMB);
		//engine->showInfo("Memory Warning", UString::format("Trying to allocate %i MB for a single buffer, but the maximum supported size is %i MB.\n", requestedMemSizeInMB, m_iMaxMemAllocSizeInMB));
	}

	cl_int ret = CL_INVALID_VALUE;
	cl_mem bufferObject = NULL;
	bufferObject = clCreateBuffer(m_context, (readable && writeable) ? CL_MEM_READ_WRITE : ( (readable && !writeable) ? CL_MEM_READ_ONLY : CL_MEM_WRITE_ONLY ), numberOfObjects * sizeof(T), NULL, &ret);

	if (ret != CL_SUCCESS)
	{
		engine->showMessageError("OpenCL Error", UString::format("clCreateBuffer() returned %i!\n", ret) );
		return -1;
	}
	if (bufferObject == NULL)
	{
		engine->showMessageError("OpenCL Error", UString::format("clCreateBuffer() returned NULL!\n", ret) );
		return -1;
	}

	m_vBuffers.push_back(bufferObject);
	m_vBufferIndex.push_back(m_vBuffers.size()-1);
	return m_vBufferIndex.size()-1;

#else
	return -1;
#endif
}

template <typename T>
void OpenCLInterface::writeBuffer(int buffer, unsigned int numberOfObjects, const void *ptr)
{
#ifdef MCENGINE_FEATURE_OPENCL

	if (buffer < 0 || buffer > (int)(m_vBufferIndex.size()-1) || numberOfObjects < 1 || ptr == NULL)
	{
		engine->showMessageError("OpenCL Error", UString::format("Illegal arguments to writeBuffer(%i, %i, [ptr])!", buffer, numberOfObjects));
		return;
	}

	if (m_vBufferIndex[buffer] > (int)(m_vBuffers.size()-1) || m_vBufferIndex[buffer] < 0)
	{
		engine->showMessageError("OpenCL Error", UString::format("Illegal arguments to writeBuffer(%i, %i,[ptr])!", m_vBufferIndex[buffer], numberOfObjects));
		return;
	}

	cl_int ret = CL_INVALID_VALUE;
	ret = clEnqueueWriteBuffer(m_commandQueue, m_vBuffers[m_vBufferIndex[buffer]], CL_TRUE, 0, numberOfObjects * sizeof(T), ptr, 0, NULL, NULL);

	if (ret != CL_SUCCESS)
		engine->showMessageError("OpenCL Error", UString::format("clEnqueueWriteBuffer() returned %i!\n\n(You can quit the engine by closing the console!)", ret) );

#endif
}

template <typename T>
void OpenCLInterface::writeBuffer(int buffer, unsigned int numberOfObjects, unsigned int startObjectIndex, const void *ptr)
{
#ifdef MCENGINE_FEATURE_OPENCL

	if (buffer < 0 || buffer > (int)(m_vBufferIndex.size()-1) || numberOfObjects < 1 || ptr == NULL || startObjectIndex < 0)
	{
		engine->showMessageError("OpenCL Error", UString::format("Illegal arguments to writeBuffer(%i, %i, %i, [ptr])!", buffer, numberOfObjects, startObjectIndex));
		return;
	}

	if (m_vBufferIndex[buffer] > (int)(m_vBuffers.size()-1) || m_vBufferIndex[buffer] < 0)
	{
		engine->showMessageError("OpenCL Error", UString::format("Illegal arguments to writeBuffer(%i, %i, %i, [ptr])!", m_vBufferIndex[buffer], numberOfObjects, startObjectIndex));
		return;
	}

	cl_int ret = CL_INVALID_VALUE;
	ret = clEnqueueWriteBuffer(m_commandQueue, m_vBuffers[m_vBufferIndex[buffer]], CL_TRUE, startObjectIndex * sizeof(T), numberOfObjects * sizeof(T), ptr, 0, NULL, NULL);

	if (ret != CL_SUCCESS)
		engine->showMessageError("OpenCL Error", UString::format("clEnqueueWriteBuffer() returned %i!\n\n(You can quit the engine by closing the console!)", ret) );

#endif
}

template <typename T>
void OpenCLInterface::readBuffer(int buffer, unsigned int numberOfObjects, void *ptr)
{
#ifdef MCENGINE_FEATURE_OPENCL

	if (buffer < 0 || buffer > (int)(m_vBufferIndex.size()-1) || numberOfObjects < 1 || ptr == NULL)
	{
		engine->showMessageError("OpenCL Error", UString::format("Illegal arguments to readBuffer(%i, [ptr])!", buffer, numberOfObjects));
		return;
	}

	if (m_vBufferIndex[buffer] > (int)(m_vBuffers.size()-1) || m_vBufferIndex[buffer] < 0)
	{
		engine->showMessageError("OpenCL Error", UString::format("Illegal arguments to readBuffer(%i, [ptr])!", m_vBufferIndex[buffer], numberOfObjects));
		return;
	}

	cl_int ret = CL_INVALID_VALUE;
	ret = clEnqueueReadBuffer(m_commandQueue, m_vBuffers[m_vBufferIndex[buffer]], CL_TRUE, 0, numberOfObjects * sizeof(T), ptr, 0, NULL, NULL);

	if (ret != CL_SUCCESS)
		engine->showMessageError("OpenCL Error", UString::format("clEnqueueReadBuffer() returned %i!\nProbably an out-of-bounds memory read/write in the kernel!!!", ret) );

#endif
}

template <typename T>
void OpenCLInterface::updateBuffer(int buffer, unsigned int numberOfObjects, bool readable, bool writeable)
{
#ifdef MCENGINE_FEATURE_OPENCL

	if (buffer < 0 || buffer > (int)(m_vBufferIndex.size()-1) || numberOfObjects < 1)
	{
		engine->showMessageError("OpenCL Error", UString::format("Illegal arguments to updateBuffer(%i, %i)!", buffer, numberOfObjects));
		return;
	}

	if (m_vBufferIndex[buffer] < 0 || m_vBufferIndex[buffer] > (int)(m_vBuffers.size()-1))
	{
		engine->showMessageError("OpenCL Error", UString::format("Illegal arguments to updateBuffer(%i, %i)!", buffer, numberOfObjects));
		return;
	}

	cl_int ret = CL_INVALID_VALUE;
	cl_mem bufferObject = NULL;
	bufferObject = clCreateBuffer(m_context, (readable && writeable) ? CL_MEM_READ_WRITE : ( (readable && !writeable) ? CL_MEM_READ_ONLY : CL_MEM_WRITE_ONLY ), numberOfObjects * sizeof(T), NULL, &ret);

	if (ret != CL_SUCCESS)
	{
		engine->showMessageError("OpenCL Error", UString::format("clCreateBuffer() returned %i!\n\n(You can quit the engine by closing the console!)", ret) );
		return;
	}
	if (bufferObject == NULL)
	{
		engine->showMessageError("OpenCL Error", UString::format("clCreateBuffer() returned NULL!\n", ret) );
		return;
	}

	// set new buffer
	m_vBuffers[m_vBufferIndex[buffer]] = bufferObject;

#endif
}

template <typename T>
void OpenCLInterface::setKernelArg(int kernel, unsigned int argumentNumber, T argument)
{
#ifdef MCENGINE_FEATURE_OPENCL

	if (kernel < 0 || kernel > (int)(m_vKernels.size()-1) || argumentNumber < 0)
	{
		engine->showMessageError("OpenCL Error", UString::format("Illegal arguments to setKernelArg(%i, %i, T [template])!", kernel, argumentNumber));
		return;
	}

	cl_int ret = CL_INVALID_VALUE;
	ret = clSetKernelArg(m_vKernels[kernel], argumentNumber, sizeof(T), (void *)&argument);

	if (ret != CL_SUCCESS)
		engine->showMessageError("OpenCL Error", UString::format("clSetKernelArg(%i, %i, T [template]) returned %i!\n\n(You can quit the engine by closing the console!)", kernel, argumentNumber, ret));

#endif
}



// helper functions

inline size_t shrRoundUp(int group_size, int global_size)
{
	int r = global_size % group_size;
	return r == 0 ? global_size : global_size + group_size - r;
}

#endif
