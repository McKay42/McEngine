//================ Copyright (c) 2016, PG, All rights reserved. =================//
//
// Purpose:		raw legacy opengl graphics interface
//
// $NoKeywords: $lgli
//===============================================================================//

#include "OpenGLLegacyInterface.h"

#ifdef MCENGINE_FEATURE_OPENGL

#include "Engine.h"
#include "ConVar.h"
#include "Camera.h"

#include "Font.h"
#include "OpenGLImage.h"
#include "OpenGLRenderTarget.h"
#include "OpenGLShader.h"
#include "OpenGLVertexArrayObject.h"

#include "OpenGLHeaders.h"

#define GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX          0x9047
#define GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX    0x9048
#define GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX  0x9049
#define GPU_MEMORY_INFO_EVICTION_COUNT_NVX            0x904A
#define GPU_MEMORY_INFO_EVICTED_MEMORY_NVX            0x904B

#define VBO_FREE_MEMORY_ATI                     0x87FB
#define TEXTURE_FREE_MEMORY_ATI                 0x87FC
#define RENDERBUFFER_FREE_MEMORY_ATI            0x87FD

ConVar r_image_unbind_after_drawimage("r_image_unbind_after_drawimage", true);

OpenGLLegacyInterface::OpenGLLegacyInterface() : Graphics()
{
	// renderer
	m_bInScene = false;
	m_vResolution = engine->getScreenSize(); // initial viewport size = window size

	// persistent vars
	m_bAntiAliasing = true;
	m_color = 0xffffffff;
	m_fClearZ = 1;
	m_fZ = 1;
}

void OpenGLLegacyInterface::init()
{
	// check GL version
	const GLubyte *version = glGetString(GL_VERSION);
	debugLog("OpenGL: OpenGL Version %s\n", version);

	// check GLEW
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		debugLog("glewInit() Error: %s\n", glewGetErrorString(err));
		engine->showMessageErrorFatal("OpenGL Error", "Couldn't glewInit()!\nThe engine will exit now.");
		engine->shutdown();
		return;
	}

	// check GL version again
	if (!glewIsSupported("GL_VERSION_3_0") && !glewIsSupported("GLEW_VERSION_3_0"))
		engine->showMessageWarning("OpenGL Warning", UString::format("Your GPU does not support OpenGL version 3.0!\nThe engine will try to continue, but probably crash.\nOpenGL version = %s", version));

	// enable
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glEnable(GL_COLOR_MATERIAL);

	// disable
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	// shading
	glShadeModel(GL_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	// blending
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);

	// culling
	glFrontFace(GL_CCW);

	// wireframe mode
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

OpenGLLegacyInterface::~OpenGLLegacyInterface()
{
}

void OpenGLLegacyInterface::beginScene()
{
	m_bInScene = true;

	Matrix4 defaultProjectionMatrix = Camera::buildMatrixOrtho2D(0, m_vResolution.x, m_vResolution.y, 0, -1.0f, 1.0f);

	// push main transforms
	pushTransform();
	setProjectionMatrix(defaultProjectionMatrix);
	translate(r_globaloffset_x->getFloat(), r_globaloffset_y->getFloat());

	// and apply them
	updateTransform();

	// set clear color and clear
	//glClearColor(1, 1, 1, 1);
	//glClearColor(0.9568f, 0.9686f, 0.9882f, 1);
	glClearColor(0, 0, 0, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// display any errors of previous frames
	handleGLErrors();
}

void OpenGLLegacyInterface::endScene()
{
	popTransform();

	checkStackLeaks();

	if (m_clipRectStack.size() > 0)
	{
		engine->showMessageErrorFatal("ClipRect Stack Leak", "Make sure all push*() have a pop*()!");
		engine->shutdown();
	}

	m_bInScene = false;
}

void OpenGLLegacyInterface::clearDepthBuffer()
{
	glClear(GL_DEPTH_BUFFER_BIT);
}

void OpenGLLegacyInterface::setColor(Color color)
{
	if (color == m_color) return;

	m_color = color;
	//glColor4f(((unsigned char)(m_color >> 16))  / 255.0f, ((unsigned char)(m_color >> 8)) / 255.0f, ((unsigned char)(m_color >> 0)) / 255.0f, ((unsigned char)(m_color >> 24)) / 255.0f);
	glColor4ub((unsigned char)(m_color >> 16), (unsigned char)(m_color >> 8), (unsigned char)(m_color >> 0), (unsigned char)(m_color >> 24));
}

void OpenGLLegacyInterface::setAlpha(float alpha)
{
	Color tempColor = m_color;

	tempColor &= 0x00ffffff;
	tempColor |= ((int)(255.0f * alpha)) << 24;

	setColor(tempColor);
}

void OpenGLLegacyInterface::drawPixels(int x, int y, int width, int height, Graphics::DRAWPIXELS_TYPE type, const void *pixels)
{
	glRasterPos2i(x, y + height); // '+height' because of opengl bottom left origin, but engine top left origin
	glDrawPixels(width, height, GL_RGBA, (type == Graphics::DRAWPIXELS_TYPE::DRAWPIXELS_UBYTE ? GL_UNSIGNED_BYTE : GL_FLOAT), pixels);
}

void OpenGLLegacyInterface::drawPixel(int x, int y)
{
	updateTransform();

	glDisable(GL_TEXTURE_2D);

	glBegin(GL_POINTS);
	{
		glVertex2i(x, y);
	}
	glEnd();
}

void OpenGLLegacyInterface::drawLine(int x1, int y1, int x2, int y2)
{
	updateTransform();

	glDisable(GL_TEXTURE_2D);

	glBegin(GL_LINES);
	{
		glVertex2f(x1 + 0.5f, y1 + 0.5f);
		glVertex2f(x2 + 0.5f, y2 + 0.5f);
	}
	glEnd();
}

void OpenGLLegacyInterface::drawLine(Vector2 pos1, Vector2 pos2)
{
	drawLine(pos1.x, pos1.y, pos2.x, pos2.y);
}

void OpenGLLegacyInterface::drawRect(int x, int y, int width, int height)
{
	updateTransform();

	glDisable(GL_TEXTURE_2D);

	glBegin(GL_LINES);
	{
		glVertex2f((x + 1) + 0.5f, y + 0.5f);
		glVertex2f((x + width) + 0.5f, y + 0.5f);

		glVertex2f(x + 0.5f, y + 0.5f);
		glVertex2f(x + 0.5f, (y + height) + 0.5f);

		glVertex2f(x + 0.5f, (y + height) + 0.5f);
		glVertex2f((x + width + 1) + 0.5f, (y + height) + 0.5f);

		glVertex2f((x + width) + 0.5f, y + 0.5f);
		glVertex2f((x + width) + 0.5f, (y + height) + 0.5f);
	}
	glEnd();
}

void OpenGLLegacyInterface::drawRect(int x, int y, int width, int height, Color top, Color right, Color bottom, Color left)
{
	updateTransform();

	glDisable(GL_TEXTURE_2D);

	glBegin(GL_LINES);
	{
		setColor(top);
		glVertex2f((x + 1) + 0.5f, y + 0.5f);
		glVertex2f((x + width) + 0.5f, y + 0.5f);

		setColor(left);
		glVertex2f(x + 0.5f, y + 0.5f);
		glVertex2f(x + 0.5f, (y + height) + 0.5f);

		setColor(bottom);
		glVertex2f(x + 0.5f, (y + height) + 0.5f);
		glVertex2f((x + width + 1) + 0.5f, (y + height) + 0.5f);

		setColor(right);
		glVertex2f((x + width) + 0.5f, y + 0.5f);
		glVertex2f((x + width) + 0.5f, (y + height) + 0.5f);
	}
	glEnd();
}

void OpenGLLegacyInterface::fillRect(int x, int y, int width, int height)
{
	updateTransform();

	glDisable(GL_TEXTURE_2D);

	glBegin(GL_QUADS);
	{
		glVertex2i(x, y);
		glVertex2i(x, (y + height));
		glVertex2i((x + width), (y + height));
		glVertex2i((x + width), y);
	}
	glEnd();
}

void OpenGLLegacyInterface::fillRoundedRect(int x, int y, int width, int height, int radius)
{
	float xOffset = x + radius;
	float yOffset = y + radius;

	double i = 0;
	const double factor = 0.05;

	updateTransform();

	glDisable(GL_TEXTURE_2D);

	glBegin(GL_POLYGON);
	{
		// top left
		for (i=PI; i<=(1.5*PI); i+=factor)
		{
			glVertex2d(radius* std::cos(i) + xOffset, radius * std::sin(i) + yOffset);
		}

		// top right
		xOffset = x + width - radius;
		for (i=(1.5*PI); i<=(2*PI); i+=factor)
		{
			glVertex2d(radius* std::cos(i) + xOffset, radius * std::sin(i) + yOffset);
		}

		// bottom right
		yOffset = y + height - radius;
		for (i=0; i<=(0.5*PI); i+=factor)
		{
			glVertex2d(radius* std::cos(i) + xOffset, radius * std::sin(i) + yOffset);
		}

		// bottom left
		xOffset = x + radius;
		for (i=(0.5*PI); i<=PI; i+=factor)
		{
			glVertex2d(radius* std::cos(i) + xOffset, radius * std::sin(i) + yOffset);
		}
	}
	glEnd();
}

void OpenGLLegacyInterface::fillGradient(int x, int y, int width, int height, Color topLeftColor, Color topRightColor, Color bottomLeftColor, Color bottomRightColor)
{
	updateTransform();

	glDisable(GL_TEXTURE_2D);

	glBegin(GL_QUADS);
	{
		setColor(topLeftColor);
		glVertex2i(x, y);

		setColor(topRightColor);
		glVertex2i((x + width), y);

		setColor(bottomRightColor);
		glVertex2i((x + width), (y + height));

		setColor(bottomLeftColor);
		glVertex2i(x, (y + height));
	}
	glEnd();
}

void OpenGLLegacyInterface::drawQuad(int x, int y, int width, int height)
{
	updateTransform();

	glBegin(GL_QUADS);
	{
		glTexCoord2f(0, 0);
		glVertex2f(x, y);

		glTexCoord2f(0, 1);
		glVertex2f(x, (y + height));

		glTexCoord2f(1, 1);
		glVertex2f((x + width), (y + height));

		glTexCoord2f(1, 0);
		glVertex2f((x + width), y);
	}
	glEnd();
}

void OpenGLLegacyInterface::drawQuad(Vector2 topLeft, Vector2 topRight, Vector2 bottomRight, Vector2 bottomLeft, Color topLeftColor, Color topRightColor, Color bottomRightColor, Color bottomLeftColor)
{
	updateTransform();

	glBegin(GL_QUADS);
	{
		setColor(topLeftColor);
		glTexCoord2f(0, 0);
		glVertex2f(topLeft.x, topLeft.y);

		setColor(bottomLeftColor);
		glTexCoord2f(0, 1);
		glVertex2f(bottomLeft.x, bottomLeft.y);

		setColor(bottomRightColor);
		glTexCoord2f(1, 1);
		glVertex2f(bottomRight.x, bottomRight.y);

		setColor(topRightColor);
		glTexCoord2f(1, 0);
		glVertex2f(topRight.x, topRight.y);
	}
	glEnd();
}

void OpenGLLegacyInterface::drawImage(Image *image)
{
	if (image == NULL)
	{
		debugLog("WARNING: Tried to draw image with NULL texture!\n");
		return;
	}
	if (!image->isReady()) return;

	updateTransform();

	const float width = image->getWidth();
	const float height = image->getHeight();

	const float x = -width/2;
	const float y = -height/2;

	image->bind();
	{
		glBegin(GL_QUADS);
		{
			glTexCoord2f(0, 0);
			glVertex2f(x, y);

			glTexCoord2f(0, 1);
			glVertex2f(x, (y + height));

			glTexCoord2f(1, 1);
			glVertex2f((x + width), (y + height));

			glTexCoord2f(1, 0);
			glVertex2f((x + width), y);
		}
		glEnd();
	}
	if (r_image_unbind_after_drawimage.getBool())
		image->unbind();

	if (r_debug_drawimage->getBool())
	{
		setColor(0xbbff00ff);
		drawRect(x, y, width, height);
	}
}

void OpenGLLegacyInterface::drawString(McFont *font, UString text)
{
	if (font == NULL || text.length() < 1 || !font->isReady()) return;

	updateTransform();

	if (r_debug_flush_drawstring->getBool())
	{
		glFinish();
		glFlush();
		glFinish();
		glFlush();
	}

	font->drawString(this, text);
}

void OpenGLLegacyInterface::drawVAO(VertexArrayObject *vao)
{
	if (vao == NULL) return;

	updateTransform();

	// HACKHACK: disable texturing for special primitives, also for untextured vaos
	if (vao->getPrimitive() == Graphics::PRIMITIVE::PRIMITIVE_LINES || vao->getPrimitive() == Graphics::PRIMITIVE::PRIMITIVE_LINE_STRIP || !vao->hasTexcoords())
		glDisable(GL_TEXTURE_2D);

	// if baked, then we can directly draw the buffer
	if (vao->isReady())
	{
		((OpenGLVertexArrayObject*)vao)->draw();
		return;
	}

	const std::vector<Vector3> &vertices = vao->getVertices();
	const std::vector<Vector3> &normals = vao->getNormals();
	const std::vector<std::vector<Vector2>> &texcoords = vao->getTexcoords();
	const std::vector<Color> &colors = vao->getColors();

	glBegin(primitiveToOpenGL(vao->getPrimitive()));
	for (size_t i=0; i<vertices.size(); i++)
	{
		if (i < colors.size())
			setColor(colors[i]);

		for (size_t t=0; t<texcoords.size(); t++)
		{
			if (i < texcoords[t].size())
				glMultiTexCoord2f(GL_TEXTURE0 + t, texcoords[t][i].x, texcoords[t][i].y);
		}

		if (i < normals.size())
			glNormal3f(normals[i].x, normals[i].y, normals[i].z);

		glVertex3f(vertices[i].x, vertices[i].y, vertices[i].z);
	}
	glEnd();
}

void OpenGLLegacyInterface::setClipRect(McRect clipRect)
{
	if (r_debug_disable_cliprect->getBool()) return;
	//if (m_bIs3DScene) return; // HACKHACK:TODO:

	// HACKHACK: compensate for viewport changes caused by RenderTargets!
	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	//debugLog("viewport = %i, %i, %i, %i\n", viewport[0], viewport[1], viewport[2], viewport[3]);

	glEnable(GL_SCISSOR_TEST);
	glScissor((int)clipRect.getX()+viewport[0], viewport[3]-((int)clipRect.getY()-viewport[1]-1+(int)clipRect.getHeight()), (int)clipRect.getWidth(), (int)clipRect.getHeight());

	//debugLog("scissor = %i, %i, %i, %i\n", (int)clipRect.getX()+viewport[0], viewport[3]-((int)clipRect.getY()-viewport[1]-1+(int)clipRect.getHeight()), (int)clipRect.getWidth(), (int)clipRect.getHeight());
}

void OpenGLLegacyInterface::pushClipRect(McRect clipRect)
{
	if (m_clipRectStack.size() > 0)
		m_clipRectStack.push(m_clipRectStack.top().intersect(clipRect));
	else
		m_clipRectStack.push(clipRect);

	setClipRect(m_clipRectStack.top());
}

void OpenGLLegacyInterface::popClipRect()
{
	m_clipRectStack.pop();

	if (m_clipRectStack.size() > 0)
		setClipRect(m_clipRectStack.top());
	else
		setClipping(false);
}

void OpenGLLegacyInterface::pushStencil()
{
	// init and clear
	glClearStencil(0);
	glClear(GL_STENCIL_BUFFER_BIT);
	glEnable(GL_STENCIL_TEST);

	// set mask
	glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
	glStencilFunc( GL_ALWAYS, 1, 1 );
	glStencilOp( GL_REPLACE, GL_REPLACE, GL_REPLACE );
}

void OpenGLLegacyInterface::fillStencil(bool inside)
{
	glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
	glStencilFunc( GL_NOTEQUAL, inside ? 0 : 1, 1 );
	glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
}

void OpenGLLegacyInterface::popStencil()
{
	glDisable(GL_STENCIL_TEST);
}

void OpenGLLegacyInterface::setClipping(bool enabled)
{
	if (enabled)
	{
		if (m_clipRectStack.size() > 0)
			glEnable(GL_SCISSOR_TEST);
	}
	else
		glDisable(GL_SCISSOR_TEST);
}

void OpenGLLegacyInterface::setAlphaTesting(bool enabled)
{
	if (enabled)
		glEnable(GL_ALPHA_TEST);
	else
		glDisable(GL_ALPHA_TEST);
}

void OpenGLLegacyInterface::setAlphaTestFunc(COMPARE_FUNC alphaFunc, float ref)
{
	glAlphaFunc(compareFuncToOpenGL(alphaFunc), ref);
}

void OpenGLLegacyInterface::setBlending(bool enabled)
{
	if (enabled)
		glEnable(GL_BLEND);
	else
		glDisable(GL_BLEND);
}

void OpenGLLegacyInterface::setBlendMode(BLEND_MODE blendMode)
{
	switch (blendMode)
	{
	case BLEND_MODE::BLEND_MODE_ALPHA:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;
	case BLEND_MODE::BLEND_MODE_ADDITIVE:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		break;
	case BLEND_MODE::BLEND_MODE_PREMUL_ALPHA:
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		break;
	case BLEND_MODE::BLEND_MODE_PREMUL_COLOR:
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		break;
	}
}

void OpenGLLegacyInterface::setDepthBuffer(bool enabled)
{
	if (enabled)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);
}

void OpenGLLegacyInterface::setCulling(bool culling)
{
	if (culling)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);
}

void OpenGLLegacyInterface::setAntialiasing(bool aa)
{
	m_bAntiAliasing = aa;
	if (aa)
		glEnable(GL_MULTISAMPLE);
	else
		glDisable(GL_MULTISAMPLE);
}

void OpenGLLegacyInterface::setWireframe(bool enabled)
{
	if (enabled)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void OpenGLLegacyInterface::flush()
{
	glFlush();
}

std::vector<unsigned char> OpenGLLegacyInterface::getScreenshot()
{
	std::vector<unsigned char> result;
	unsigned int width = m_vResolution.x;
	unsigned int height = m_vResolution.y;

	// sanity check
	if (width > 65535 || height > 65535 || width < 1 || height < 1)
	{
		engine->showMessageError("Renderer Error", "getScreenshot() called with invalid arguments!");
		return result;
	}

	unsigned int numElements = width*height*3;

	// take screenshot
	unsigned char *pixels = new unsigned char[numElements];
	glFinish();
	for (int y=0; y<height; y++) // flip it while reading
	{
		glReadPixels(0, (height - (y + 1)), width, 1, GL_RGB, GL_UNSIGNED_BYTE, &(pixels[y*width*3]));
	}

	// copy to vector
	result.resize(numElements);
	result.assign(pixels, pixels + numElements);
	delete[] pixels;

	return result;
}

UString OpenGLLegacyInterface::getVendor()
{
	const GLubyte *vendor = glGetString(GL_VENDOR);
	return reinterpret_cast<const char*>(vendor);
}

UString OpenGLLegacyInterface::getModel()
{
	const GLubyte *model = glGetString(GL_RENDERER);
	return reinterpret_cast<const char*>(model);
}

UString OpenGLLegacyInterface::getVersion()
{
	const GLubyte *version = glGetString(GL_VERSION);
	return reinterpret_cast<const char*>(version);
}

int OpenGLLegacyInterface::getVRAMTotal()
{
	int nvidiaMemory = -1;
	int atiMemory = -1;

	glGetIntegerv(GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &nvidiaMemory);
	glGetIntegerv(TEXTURE_FREE_MEMORY_ATI, &atiMemory);

	glGetError(); // clear error state

	if (nvidiaMemory < 1)
		return atiMemory;
	else
		return nvidiaMemory;
}

int OpenGLLegacyInterface::getVRAMRemaining()
{
	int nvidiaMemory = -1;
	int atiMemory = -1;

	glGetIntegerv(GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &nvidiaMemory);
	glGetIntegerv(TEXTURE_FREE_MEMORY_ATI, &atiMemory);

	glGetError(); // clear error state

	if (nvidiaMemory < 1)
		return atiMemory;
	else
		return nvidiaMemory;
}

void OpenGLLegacyInterface::onResolutionChange(Vector2 newResolution)
{
	// rebuild viewport
	m_vResolution = newResolution;
	glViewport(0, 0, m_vResolution.x, m_vResolution.y);

	// special case: custom rendertarget resolution rendering, update active projection matrix immediately
	if (m_bInScene)
	{
		m_projectionTransformStack.top() = Camera::buildMatrixOrtho2D(0, m_vResolution.x, m_vResolution.y, 0, -1.0f, 1.0f);
		m_bTransformUpToDate = false;
	}
}

Image *OpenGLLegacyInterface::createImage(UString filePath, bool mipmapped, bool keepInSystemMemory)
{
	return new OpenGLImage(filePath, mipmapped, keepInSystemMemory);
}

Image *OpenGLLegacyInterface::createImage(int width, int height, bool mipmapped, bool keepInSystemMemory)
{
	return new OpenGLImage(width, height, mipmapped, keepInSystemMemory);
}

RenderTarget *OpenGLLegacyInterface::createRenderTarget(int x, int y, int width, int height, Graphics::MULTISAMPLE_TYPE multiSampleType)
{
	return new OpenGLRenderTarget(x, y, width, height, multiSampleType);
}

Shader *OpenGLLegacyInterface::createShaderFromFile(UString vertexShaderFilePath, UString fragmentShaderFilePath)
{
	return new OpenGLShader(vertexShaderFilePath, fragmentShaderFilePath, false);
}

Shader *OpenGLLegacyInterface::createShaderFromSource(UString vertexShader, UString fragmentShader)
{
	return new OpenGLShader(vertexShader, fragmentShader, true);
}

VertexArrayObject *OpenGLLegacyInterface::createVertexArrayObject(Graphics::PRIMITIVE primitive, Graphics::USAGE_TYPE usage, bool keepInSystemMemory)
{
	return new OpenGLVertexArrayObject(primitive, usage, keepInSystemMemory);
}

void OpenGLLegacyInterface::onTransformUpdate(Matrix4 &projectionMatrix, Matrix4 &worldMatrix)
{
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(projectionMatrix.get());

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(worldMatrix.get());
}

int OpenGLLegacyInterface::primitiveToOpenGL(Graphics::PRIMITIVE primitive)
{
	switch (primitive)
	{
	case Graphics::PRIMITIVE::PRIMITIVE_LINES:
		return GL_LINES;
	case Graphics::PRIMITIVE::PRIMITIVE_LINE_STRIP:
		return GL_LINE_STRIP;
	case Graphics::PRIMITIVE::PRIMITIVE_TRIANGLES:
		return GL_TRIANGLES;
	case Graphics::PRIMITIVE::PRIMITIVE_TRIANGLE_FAN:
		return GL_TRIANGLE_FAN;
	case Graphics::PRIMITIVE::PRIMITIVE_TRIANGLE_STRIP:
		return GL_TRIANGLE_STRIP;
	case Graphics::PRIMITIVE::PRIMITIVE_QUADS:
		return GL_QUADS;
	}

	return GL_TRIANGLES;
}

int OpenGLLegacyInterface::compareFuncToOpenGL(Graphics::COMPARE_FUNC compareFunc)
{
	switch (compareFunc)
	{
	case Graphics::COMPARE_FUNC::COMPARE_FUNC_NEVER:
		return GL_NEVER;
	case Graphics::COMPARE_FUNC::COMPARE_FUNC_LESS:
		return GL_LESS;
	case Graphics::COMPARE_FUNC::COMPARE_FUNC_EQUAL:
		return GL_EQUAL;
	case Graphics::COMPARE_FUNC::COMPARE_FUNC_LESSEQUAL:
		return GL_LEQUAL;
	case Graphics::COMPARE_FUNC::COMPARE_FUNC_GREATER:
		return GL_GREATER;
	case Graphics::COMPARE_FUNC::COMPARE_FUNC_NOTEQUAL:
		return GL_NOTEQUAL;
	case Graphics::COMPARE_FUNC::COMPARE_FUNC_GREATEREQUAL:
		return GL_GEQUAL;
	case Graphics::COMPARE_FUNC::COMPARE_FUNC_ALWAYS:
		return GL_ALWAYS;
	}

	return GL_ALWAYS;
}

void OpenGLLegacyInterface::handleGLErrors()
{
	int error = glGetError();
	if (error != 0)
		debugLog("OpenGL Error: %i on frame %i\n",error,engine->getFrameCount());
}

#endif
