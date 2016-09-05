//================ Copyright (c) 2016, PG, All rights reserved. =================//
//
// Purpose:		raw legacy opengl graphics interface
//
// $NoKeywords: $lgli
//===============================================================================//

#include <OpenGLLegacyInterface.h>
#include "Engine.h"
#include "ConVar.h"
#include "Camera.h"

#include "Image.h"
#include "Font.h"

#include "VertexArrayObject.h"
#include "VertexBuffer.h"
#include "OpenGLHeaders.h"

#define GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX          0x9047
#define GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX    0x9048
#define GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX  0x9049
#define GPU_MEMORY_INFO_EVICTION_COUNT_NVX            0x904A
#define GPU_MEMORY_INFO_EVICTED_MEMORY_NVX            0x904B

#define VBO_FREE_MEMORY_ATI                     0x87FB
#define TEXTURE_FREE_MEMORY_ATI                 0x87FC
#define RENDERBUFFER_FREE_MEMORY_ATI            0x87FD

ConVar r_globaloffset_x("r_globaloffset_x", 0.0f);
ConVar r_globaloffset_y("r_globaloffset_y", 0.0f);

ConVar r_debug_disable_cliprect("r_debug_disable_cliprect", false);
ConVar r_debug_disable_3dscene("r_debug_disable_3dscene", false);
ConVar r_debug_flush_drawstring("r_debug_flush_drawstring", false);

OpenGLLegacyInterface::OpenGLLegacyInterface()
{
	// renderer
	m_vResolution = engine->getScreenSize(); // initial viewport size = window size

	// persistent vars
	m_bAntiAliasing = true;
	m_color = 0xffffffff;
	m_fClearZ = 1;
	m_fZ = 1;

	// push identity
	m_worldTransformStack.push(Matrix4());
	m_projectionTransformStack.push(Matrix4());
	m_bTransformUpToDate = false;

	// push false
	m_3dSceneStack.push(false);
	m_bIs3dScene = false;
}

void OpenGLLegacyInterface::init()
{
	// check GL version
	const GLubyte *version = glGetString(GL_VERSION);
	debugLog("OpenGL: OpenGL Version %s\n",version);

	// check GLEW
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		debugLog("glewInit() Error: %s\n", glewGetErrorString(err));
		engine->showMessageErrorFatal("OpenGL Error", "Couldn't glewInit()!\nThe engine will exit now.");
		engine->shutdown();
	}

	// check GL version again
	if (!glewIsSupported("GL_VERSION_2_1"))
		engine->showMessageWarning("OpenGL Warning", "Your computer does not support OpenGL version 2.1!\nThe engine will try to continue, but probably crash.");

	// enable
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	///glEnable(GL_NORMALIZE); // why was this enabled again?
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
	Matrix4 defaultProjectionMatrix = Camera::buildMatrixOrtho2D(0, m_vResolution.x, m_vResolution.y, 0);

	// push main transforms
	pushTransform();
	setProjectionMatrix(defaultProjectionMatrix);
	translate(r_globaloffset_x.getFloat(), r_globaloffset_y.getFloat());

	// and apply them
	updateTransform();

	// set clear color and clear
	//glClearColor(1, 1, 1, 1);
	//glClearColor(0.9568f, 0.9686f, 0.9882f, 1);
	glClearColor(0, 0, 0, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// toggles
	if (m_bAntiAliasing)
		glEnable(GL_MULTISAMPLE);

	// display any errors of previous frames
	handleGLErrors();
}

void OpenGLLegacyInterface::endScene()
{
	popTransform();

	if (m_worldTransformStack.size() > 1)
	{
		engine->showMessageErrorFatal("World Transform Stack Leak", "Make sure all push*() have a pop*()!");
		engine->shutdown();
	}
	if (m_projectionTransformStack.size() > 1)
	{
		engine->showMessageErrorFatal("Projection Transform Stack Leak", "Make sure all push*() have a pop*()!");
		engine->shutdown();
	}
	if (m_clipRectStack.size() > 0)
	{
		engine->showMessageErrorFatal("ClipRect Stack Leak", "Make sure all push*() have a pop*()!");
		engine->shutdown();
	}
	if (m_3dSceneStack.size() > 1)
	{
		engine->showMessageErrorFatal("3DScene Stack Leak", "Make sure all push*() have a pop*()!");
		engine->shutdown();
	}
}

void OpenGLLegacyInterface::clearDepthBuffer()
{
	glClear(GL_DEPTH_BUFFER_BIT);
}

void OpenGLLegacyInterface::setColor(Color color)
{
	m_color = color;
	glColor4f(((unsigned char)(m_color >> 16))  / 255.0f, ((unsigned char)(m_color >> 8)) / 255.0f, ((unsigned char)(m_color >> 0)) / 255.0f, ((unsigned char)(m_color >> 24)) / 255.0f);
}

void OpenGLLegacyInterface::setAlpha(float alpha)
{
	m_color &= 0x00ffffff;
	m_color |= ((int)(255.0f * alpha)) << 24;
	setColor(m_color);
}

void OpenGLLegacyInterface::drawPixels(int x, int y, int width, int height, DRAWPIXELS_TYPE type, const void *pixels)
{
	glRasterPos2i(x, y+height); // '+height' because of opengl bottom left origin, but engine top left origin
	glDrawPixels(width, height, GL_RGBA, (type == DRAWPIXELS_UBYTE ? GL_UNSIGNED_BYTE : GL_FLOAT), pixels);
}

void OpenGLLegacyInterface::drawPixel(int x, int y)
{
	updateTransform();

	glDisable(GL_TEXTURE_2D);
	glBegin(GL_POINTS);
		glVertex2i( x, y );
	glEnd();
}

void OpenGLLegacyInterface::drawLine(int x1, int y1, int x2, int y2)
{
	updateTransform();

	glDisable(GL_TEXTURE_2D);
	glBegin(GL_LINES);
		glVertex2f( x1+0.5f, y1+0.5f);
		glVertex2f( x2+0.5f, y2+0.5f);
	glEnd();
}

void OpenGLLegacyInterface::drawLine(Vector2 pos1, Vector2 pos2)
{
	drawLine(pos1.x, pos1.y, pos2.x, pos2.y);
}

void OpenGLLegacyInterface::drawRect(int x, int y, int width, int height)
{
	drawLine(x, y, x+width, y);
	drawLine(x, y, x, y+height);
	drawLine(x, y+height, x+width+1, y+height);
	drawLine(x+width, y, x+width, y+height);
}

void OpenGLLegacyInterface::drawRect(int x, int y, int width, int height, Color top, Color right, Color bottom, Color left)
{
	setColor(top);
	drawLine(x, y, x+width, y);
	setColor(left);
	drawLine(x, y, x, y+height);
	setColor(bottom);
	drawLine(x, y+height, x+width+1, y+height);
	setColor(right);
	drawLine(x+width, y, x+width, y+height);
}

void OpenGLLegacyInterface::fillRect(int x, int y, int width, int height)
{
	updateTransform();

	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
		glVertex2i(x, y);
		glVertex2i(x, y+height);
		glVertex2i(x+width, y+height);
		glVertex2i(x+width, y);
	glEnd();
}

void OpenGLLegacyInterface::fillRoundedRect(int x, int y, int width, int height, int radius)
{
	float xOffset= x + radius;
	float yOffset = y + radius;

	double i = 0;
	double factor = 0.05;

	updateTransform();

	glDisable(GL_TEXTURE_2D);
	glBegin(GL_POLYGON);

		for(i=PI; i<=(1.5*PI); i+=factor)
		{
			glVertex2d(radius* std::cos(i) + xOffset, radius * std::sin(i) + yOffset); // top left
		}

		xOffset = x + width - radius;
		for(i=(1.5*PI); i<=(2*PI); i+=factor)
		{
			glVertex2d(radius* std::cos(i) + xOffset, radius * std::sin(i) + yOffset); // top right
		}

		yOffset = y + height - radius;
		for(i=0; i<=(0.5*PI); i+=factor)
		{
			glVertex2d(radius* std::cos(i) + xOffset, radius * std::sin(i) + yOffset); // bottom right
		}

		xOffset = x + radius;
		for(i=(0.5*PI); i<=PI; i+=factor)
		{
			glVertex2d(radius* std::cos(i) + xOffset, radius * std::sin(i) + yOffset); // bottom left
		}

	glEnd();
}

void OpenGLLegacyInterface::fillGradient(int x, int y, int width, int height, Color topLeftColor, Color topRightColor, Color bottomLeftColor, Color bottomRightColor)
{
	updateTransform();

	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	setColor(topLeftColor);
		glVertex2i(x, y);
	setColor(topRightColor);
		glVertex2i(x+width, y);
	setColor(bottomRightColor);
		glVertex2i(x+width, y+height);
	setColor(bottomLeftColor);
		glVertex2i(x, y+height);
	glEnd();
}

void OpenGLLegacyInterface::drawQuad(int x, int y, int width, int height)
{
	updateTransform();

	glBegin(GL_QUADS);

		setColor(m_color);
		glTexCoord2f(0, 0);
		glVertex2f(x, y);

		setColor(m_color);
		glTexCoord2f(0, 1);
		glVertex2f(x, y+height);

		setColor(m_color);
		glTexCoord2f(1, 1);
		glVertex2f(x+width, y+height);

		setColor(m_color);
		glTexCoord2f(1, 0);
		glVertex2f(x+width, y);

	glEnd();
}

void OpenGLLegacyInterface::drawQuad(Vector2 topLeft, Vector2 topRight, Vector2 bottomRight, Vector2 bottomLeft, Color topLeftColor, Color topRightColor, Color bottomRightColor, Color bottomLeftColor)
{
	updateTransform();

	glBegin(GL_QUADS);

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

	glEnd();
}

void OpenGLLegacyInterface::drawImage(Image *image)
{
	if (image == NULL)
	{
		debugLog("WARNING: Tried to draw image with NULL texture!\n");
		return;
	}
	if (!image->isReady())
		return;

	updateTransform();

	image->bind();

	float width = image->getWidth();
	float height = image->getHeight();

	float x = -width/2;
	float y = -height/2;

	glBegin(GL_QUADS);

		glTexCoord2f(0, 0);
		glVertex2f(x, y);

		glTexCoord2f(0, 1);
		glVertex2f(x, y+height);

		glTexCoord2f(1, 1);
		glVertex2f(x+width, y+height);

		glTexCoord2f(1, 0);
		glVertex2f(x+width, y);

	glEnd();

	image->unbind();
}

void OpenGLLegacyInterface::drawString(McFont *font, UString text)
{
	if (font == NULL || text.length() < 1 || !font->isReady())
		return;

	updateTransform();

	if (r_debug_flush_drawstring.getBool())
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
	const std::vector<Vector3> &vertices = vao->getVertices();
	const std::vector<Vector3> &normals = vao->getNormals();
	const std::vector<std::vector<Vector2>> &texcoords = vao->getTexcoords();
	const std::vector<Color> &colors = vao->getColors();

	updateTransform();

	GLenum type = GL_TRIANGLES;
	if (vao->getType() == VertexArrayObject::TYPE_TRIANGLE_FAN)
		type = GL_TRIANGLE_FAN;
	else if (vao->getType() == VertexArrayObject::TYPE_QUADS)
		type = GL_QUADS;

	glBegin(type);
	for (int i=0; i<vertices.size(); i++)
	{
		if (i < colors.size())
			setColor(colors[i]);

		for (int t=0; t<texcoords.size(); t++)
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

void OpenGLLegacyInterface::drawVB(VertexBuffer *vb)
{
	updateTransform();

	vb->draw(this);
}

void OpenGLLegacyInterface::translate(float x, float y, float z)
{
	m_worldTransformStack.top().translate(x, y, z);
	m_bTransformUpToDate = false;
}

void OpenGLLegacyInterface::rotate(float deg, float x, float y, float z)
{
	m_worldTransformStack.top().rotate(deg, x, y, z);
	m_bTransformUpToDate = false;
}

void OpenGLLegacyInterface::scale(float x, float y, float z)
{
	m_worldTransformStack.top().scale(x, y, z);
	m_bTransformUpToDate = false;
}

void OpenGLLegacyInterface::setWorldMatrix(Matrix4 &worldMatrix)
{
	m_worldTransformStack.pop();
	m_worldTransformStack.push(worldMatrix);
	m_bTransformUpToDate = false;
}

void OpenGLLegacyInterface::setWorldMatrixMul(Matrix4 &worldMatrix)
{
	Matrix4 newWorldMatrix = m_worldTransformStack.top() * worldMatrix;
	m_worldTransformStack.pop();
	m_worldTransformStack.push(newWorldMatrix);
	m_bTransformUpToDate = false;
}

void OpenGLLegacyInterface::setProjectionMatrix(Matrix4 &projectionMatrix)
{
	m_projectionTransformStack.pop();
	m_projectionTransformStack.push(projectionMatrix);
	m_bTransformUpToDate = false;
}

Matrix4 OpenGLLegacyInterface::getWorldMatrix()
{
	return m_worldTransformStack.top();
}

Matrix4 OpenGLLegacyInterface::getProjectionMatrix()
{
	return m_projectionTransformStack.top();
}

void OpenGLLegacyInterface::pushTransform()
{
	m_worldTransformStack.push(Matrix4(m_worldTransformStack.top()));
	m_projectionTransformStack.push(Matrix4(m_projectionTransformStack.top()));
}

void OpenGLLegacyInterface::popTransform()
{
	if (m_worldTransformStack.size() < 2)
	{
		engine->showMessageErrorFatal("World Transform Stack Underflow", "Too many pop*()s!");
		engine->shutdown();
		return;
	}
	if (m_projectionTransformStack.size() < 2)
	{
		engine->showMessageErrorFatal("Projection Transform Stack Underflow", "Too many pop*()s!");
		engine->shutdown();
		return;
	}

	m_worldTransformStack.pop();
	m_projectionTransformStack.pop();
	m_bTransformUpToDate = false;
}

void OpenGLLegacyInterface::setClipRect(Rect clipRect)
{
	if (r_debug_disable_cliprect.getBool()) return;
	//if (m_bIs3DScene) return; // HACKHACK:TODO:

	// HACKHACK: compensate for viewport changes caused by RenderTargets!
	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	//debugLog("viewport = %i, %i, %i, %i\n", viewport[0], viewport[1], viewport[2], viewport[3]);

	glEnable(GL_SCISSOR_TEST);
	glScissor((int)clipRect.getX()+viewport[0], viewport[3]-((int)clipRect.getY()-viewport[1]-1+(int)clipRect.getHeight()), (int)clipRect.getWidth(), (int)clipRect.getHeight());

	//debugLog("scissor = %i, %i, %i, %i\n", (int)clipRect.getX()+viewport[0], viewport[3]-((int)clipRect.getY()-viewport[1]-1+(int)clipRect.getHeight()), (int)clipRect.getWidth(), (int)clipRect.getHeight());
}

void OpenGLLegacyInterface::pushClipRect(Rect clipRect)
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

void OpenGLLegacyInterface::push3DScene(Rect region)
{
	if (r_debug_disable_3dscene.getBool()) return;

	// you can't yet stack 3d scenes!
	if (m_3dSceneStack.top())
	{
		m_3dSceneStack.push(false);
		return;
	}

	// reset & init
	m_v3dSceneOffset.x = m_v3dSceneOffset.y = m_v3dSceneOffset.z = 0;
	float m_fFov = 60.0f;

	// push true, set region
	m_bIs3dScene = true;
	m_3dSceneStack.push(true);
	m_3dSceneRegion = region;

	// backup transforms
	pushTransform();

	// calculate height to fit viewport angle
	float angle = (180.0f - m_fFov) / 2.0f;
	float b = (engine->getScreenHeight() / std::sin(deg2rad(m_fFov))) * std::sin(deg2rad(angle));
	float hc = std::sqrt(std::pow(b,2.0f) - std::pow((engine->getScreenHeight()/2.0f), 2.0f)); // thank mr pythagoras/monstrata

	// set projection matrix
	Matrix4 trans2 = Matrix4().translate(-1 + (region.getWidth()) / (float)engine->getScreenWidth() + (region.getX()*2) / (float)engine->getScreenWidth(), 1 - region.getHeight() / (float)engine->getScreenHeight() - (region.getY()*2) / (float)engine->getScreenHeight(), 0);
	Matrix4 projectionMatrix = trans2 * Camera::buildMatrixPerspectiveFov(deg2rad(m_fFov),((float) engine->getScreenWidth())/((float) engine->getScreenHeight()), -10.0f, 10.0f);
	m_3dSceneProjectionMatrix = projectionMatrix;

	// set world matrix
	Matrix4 trans = Matrix4().translate(-(float)region.getWidth()/2 - region.getX(), -(float)region.getHeight()/2 - region.getY(), 0);
	m_3dSceneWorldMatrix = Camera::buildMatrixLookAt(Vector3(0, 0, -hc), Vector3(0, 0, 0), Vector3(0, -1, 0)) * trans;

	// force transform update
	m_bTransformUpToDate = false;
	updateTransform();
}

void OpenGLLegacyInterface::pop3DScene()
{
	if (!m_3dSceneStack.top())
		return;

	m_3dSceneStack.pop();

	// restore transforms
	popTransform();

	m_bIs3dScene = false;
}

void OpenGLLegacyInterface::translate3DScene(float x, float y, float z)
{
	// block if we're not in a 3d scene
	if (!m_3dSceneStack.top())
		return;

	// translate directly
	m_3dSceneWorldMatrix.translate(x,y,z);

	m_bTransformUpToDate = false;
	updateTransform();
}

void OpenGLLegacyInterface::rotate3DScene(float rotx, float roty, float rotz)
{
	// block if we're not in a 3d scene
	if (!m_3dSceneStack.top())
		return;

	// first translate to the center of the 3d region, then rotate, then translate back
	Matrix4 rot;
	Vector3 centerVec = Vector3(m_3dSceneRegion.getX()+m_3dSceneRegion.getWidth()/2 + m_v3dSceneOffset.x, m_3dSceneRegion.getY()+m_3dSceneRegion.getHeight()/2 + m_v3dSceneOffset.y, m_v3dSceneOffset.z);
	rot.translate(-centerVec);

	// rotate
	if (rotx != 0)
		rot.rotateX(-rotx);
	if (roty != 0)
		rot.rotateY(-roty);
	if (rotz != 0)
		rot.rotateZ(-rotz);

	rot.translate(centerVec);

	// apply the rotation
	m_3dSceneWorldMatrix = m_3dSceneWorldMatrix * rot;

	m_bTransformUpToDate = false;
	updateTransform();
}

void OpenGLLegacyInterface::offset3DScene(float x, float y, float z)
{
	m_v3dSceneOffset = Vector3(x,y,z);
}

void OpenGLLegacyInterface::updateTransform()
{
	if (!m_bTransformUpToDate)
	{
		Matrix4 worldMatrixTemp = m_worldTransformStack.top();
		Matrix4 projectionMatrixTemp = m_projectionTransformStack.top();

		// 3d gui scenes
		if (m_bIs3dScene)
		{
			worldMatrixTemp = m_3dSceneWorldMatrix * m_worldTransformStack.top();
			projectionMatrixTemp = m_3dSceneProjectionMatrix;
		}

		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(projectionMatrixTemp.getTranspose());

		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(worldMatrixTemp.getTranspose());

		m_bTransformUpToDate = true;
	}
}

void OpenGLLegacyInterface::onResolutionChange(Vector2 newResolution)
{
	// rebuild viewport
	m_vResolution = newResolution;
	glViewport(0, 0, m_vResolution.x, m_vResolution.y);
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

void OpenGLLegacyInterface::setBlending(bool enabled)
{
	if (enabled)
		glEnable(GL_BLEND);
	else
		glDisable(GL_BLEND);
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

void OpenGLLegacyInterface::handleGLErrors()
{
	int error = glGetError();
	if (error != 0)
		debugLog("OpenGL Error: %i on frame %i\n",error,engine->getFrameCount());
}



//*************************************//
//	OpenGLLegacyInterface ConCommands  //
//*************************************//

void _vsync(UString oldValue, UString newValue)
{
	if (newValue.length() < 1)
		debugLog("Usage: 'vsync 1' to turn vsync on, 'vsync 0' to turn vsync off\n");
	else
	{
		bool vsync = newValue.toFloat() > 0.0f;
		engine->getGraphics()->setVSync(vsync);
	}
}
void _mat_wireframe(UString oldValue, UString newValue)
{
	if (newValue.toFloat() > 0.0f)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

ConVar __mat_wireframe("mat_wireframe", false, _mat_wireframe);
ConVar __vsync("vsync", false, _vsync);
