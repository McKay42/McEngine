//================ Copyright (c) 2017, PG, All rights reserved. =================//
//
// Purpose:		empty renderer, for debugging and new OS implementations
//
// $NoKeywords: $ni
//===============================================================================//

#include "NullGraphicsInterface.h"
#include "Engine.h"
#include "VertexArrayObject.h"

#include "NullImage.h"
#include "NullRenderTarget.h"
#include "NullShader.h"

Image *NullGraphicsInterface::createImage(UString filePath, bool mipmapped, bool keepInSystemMemory)
{
	return new NullImage(filePath, mipmapped, keepInSystemMemory);
}

Image *NullGraphicsInterface::createImage(int width, int height, bool mipmapped, bool keepInSystemMemory)
{
	return new NullImage(width, height, mipmapped, keepInSystemMemory);
}

RenderTarget *NullGraphicsInterface::createRenderTarget(int x, int y, int width, int height, Graphics::MULTISAMPLE_TYPE multiSampleType)
{
	return new NullRenderTarget(x, y, width, height, multiSampleType);
}

Shader *NullGraphicsInterface::createShaderFromFile(UString vertexShaderFilePath, UString fragmentShaderFilePath)
{
	return new NullShader(vertexShaderFilePath, fragmentShaderFilePath, false);
}

Shader *NullGraphicsInterface::createShaderFromSource(UString vertexShader, UString fragmentShader)
{
	return new NullShader(vertexShader, fragmentShader, true);
}

VertexArrayObject *NullGraphicsInterface::createVertexArrayObject(Graphics::PRIMITIVE primitive, Graphics::USAGE_TYPE usage, bool keepInSystemMemory)
{
	return new VertexArrayObject(primitive, usage, keepInSystemMemory);
}

void NullGraphicsInterface::drawString(McFont *font, UString text) {;}
UString NullGraphicsInterface::getVendor() {return "<NULL>";}
UString NullGraphicsInterface::getModel() {return "<NULL>";}
UString NullGraphicsInterface::getVersion() {return "<NULL>";}
