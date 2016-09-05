//================ Copyright (c) 2013, PG, All rights reserved. =================//
//
// Purpose:		prepares a gauss kernel to use with the blur shader
//
// $NoKeywords: $gblur
//===============================================================================//

#include "GaussianBlurKernel.h"

#include "Engine.h"

GaussianBlurKernel::GaussianBlurKernel(int kernelSize, float radius, int targetWidth, int targetHeight)
{
	m_iKernelSize = kernelSize;
	m_fRadius = radius;
	m_iTargetWidth = targetWidth;
	m_iTargetHeight = targetHeight;

	build();
}

GaussianBlurKernel::~GaussianBlurKernel()
{
	release();
}

void GaussianBlurKernel::build()
{
	// allocate kernel
	m_kernel.reserve(m_iKernelSize);

	// allocate offsets
	m_offsetsHorizontal.reserve(m_iKernelSize);
	m_offsetsVertical.reserve(m_iKernelSize);

	// calculate kernel
	int center = m_iKernelSize/2;
	double sum = 0.0f;
	double result = 0.0f;
	double _sigma = m_fRadius;
	double sigmaRoot = (double)std::sqrt( 2 * _sigma*_sigma * PI );

	// dummy fill kernel
	for (int i=0; i<m_iKernelSize; i++)
	{
		m_kernel.push_back(0.0f);
	}

	// now set the real values
	for(int i=0; i<center; i++)
    {
        result = exp( -(i*i)/(double)(2 * _sigma*_sigma) ) / sigmaRoot;
        m_kernel[center+i] = m_kernel[center-i] = (float)result;
        sum += result;
        if (i != 0)
			sum += result;
    }

	// normalize kernel
	for(int i=0; i<center; i++)
	{
		m_kernel[center+i] = m_kernel[center-i] /= (float)sum;
	}

	// calculate offsets
	double xInc = 1.0 / (double)m_iTargetWidth;
	double yInc = 1.0 / (double)m_iTargetHeight;

	for (int i=-center; i<center; i++)
	{
		m_offsetsHorizontal.push_back((float)(i * xInc));
		m_offsetsVertical.push_back((float)(i * yInc));
	}
}

void GaussianBlurKernel::release()
{
	m_kernel = std::vector<float>();
	m_offsetsHorizontal = std::vector<float>();
	m_offsetsVertical = std::vector<float>();
}
