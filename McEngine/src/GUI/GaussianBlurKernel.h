//================ Copyright (c) 2013, PG, All rights reserved. =================//
//
// Purpose:		prepares a gauss kernel to use with the blur shader
//
// $NoKeywords: $gblur
//===============================================================================//

#ifndef GAUSSIANBLURKERNEL_H
#define GAUSSIANBLURKERNEL_H

#include "cbase.h"

class GaussianBlurKernel
{
public:
	GaussianBlurKernel(int kernelSize, float radius, int targetWidth, int targetHeight);
	~GaussianBlurKernel();

	void rebuild(){release();build();}
	void release();

	inline const int getKernelSize() {return m_iKernelSize;}
	inline const float getRadius() {return m_fRadius;}

	inline float* getKernel() {return &m_kernel.front();}
	inline float* getOffsetsHorizontal() {return &m_offsetsHorizontal.front();}
	inline float* getOffsetsVertical() {return &m_offsetsVertical.front();}

private:
	void build();

	float m_fRadius;
	int m_iKernelSize;
	int m_iTargetWidth,m_iTargetHeight;

	std::vector<float> m_kernel;
	std::vector<float> m_offsetsHorizontal;
	std::vector<float> m_offsetsVertical;
};

#endif
