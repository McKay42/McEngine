//================ Copyright (c) 2017, PG, All rights reserved. =================//
//
// Purpose:		empty renderer, for debugging and new OS implementations
//
// $NoKeywords: $ni
//===============================================================================//

#include "NullGraphicsInterface.h"
#include "Engine.h"

#include "Image.h"
#include "RenderTarget.h"
#include "Shader.h"

class NullImage : public Image
{
public:
	NullImage(UString filePath, bool mipmapped) : Image(filePath, mipmapped) {;}
	NullImage(int width, int height, bool clampToEdge) : Image(width, height, clampToEdge) {;}
	virtual ~NullImage() {destroy();}

	virtual void bind(unsigned int textureUnit = 0) {;}
	virtual void unbind() {;}

	virtual void setFilterMode(Graphics::FILTER_MODE filterMode) {;}
	virtual void setWrapMode(Graphics::WRAP_MODE wrapMode) {;}

private:
	virtual void init() {m_bReady = true;}
	virtual void initAsync() {m_bAsyncReady = true;}
	virtual void destroy() {;}
};

class NullRenderTarget : public RenderTarget
{
public:
	NullRenderTarget(int x, int y, int width, int height, Graphics::MULTISAMPLE_TYPE multiSampleType) : RenderTarget(x, y, width, height, multiSampleType) {;}
	virtual ~NullRenderTarget() {destroy();}

	virtual void enable() {;}
	virtual void disable() {;}

	virtual void bind(unsigned int textureUnit = 0) {;}
	virtual void unbind() {;}

private:
	virtual void init() {m_bReady = true;}
	virtual void initAsync() {m_bAsyncReady = true;}
	virtual void destroy() {;}
};

class NullShader : public Shader
{
public:
	NullShader(UString vertexShader, UString fragmentShader, bool source) : Shader() {;}
	virtual ~NullShader() {destroy();}

	virtual void enable() {;}
	virtual void disable() {;}

	virtual void setUniform1f(UString name, float value) {;}
	virtual void setUniform1fv(UString name, int count, float *values) {;}
	virtual void setUniform1i(UString name, int value) {;}
	virtual void setUniform2f(UString name, float x, float y) {;}
	virtual void setUniform2fv(UString name, int count, float *vectors) {;}
	virtual void setUniform3f(UString name, float x, float y, float z) {;}
	virtual void setUniform3fv(UString name, int count, float *vectors) {;}
	virtual void setUniformMatrix4fv(UString name, Matrix4 &matrix) {;}
	virtual void setUniformMatrix4fv(UString name, float *v) {;}

private:
	virtual void init() {m_bReady = true;}
	virtual void initAsync() {m_bAsyncReady = true;}
	virtual void destroy() {;}
};

Image *NullGraphicsInterface::createImage(UString filePath, bool mipmapped)
{
	return new NullImage(filePath, mipmapped);
}

Image *NullGraphicsInterface::createImage(int width, int height, bool clampToEdge)
{
	return new NullImage(width, height, clampToEdge);
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

void NullGraphicsInterface::drawString(McFont *font, UString text) {;}
UString NullGraphicsInterface::getVendor() {return "<NULL>";}
UString NullGraphicsInterface::getModel() {return "<NULL>";}
UString NullGraphicsInterface::getVersion() {return "<NULL>";}
