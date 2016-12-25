//================ Copyright (c) 2016, PG, All rights reserved. =================//
//
// Purpose:		software renderer, just for fun
//
// $NoKeywords: $swi
//===============================================================================//

#include "SWGraphicsInterface.h"

#include "Engine.h"
#include "ConVar.h"
#include "Camera.h"

#include "Font.h"
#include "SWImage.h"
#include "SWRenderTarget.h"
#include "SWShader.h"

#include "VertexArrayObject.h"

SWGraphicsInterface::SWGraphicsInterface()
{
	// renderer
	m_vResolution = engine->getScreenSize(); // initial viewport size = window size
	m_backBuffer = new PIXEL[(int)(m_vResolution.x*m_vResolution.y)];

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

void SWGraphicsInterface::init()
{

}

SWGraphicsInterface::~SWGraphicsInterface()
{
	if (m_backBuffer != NULL)
		delete[] m_backBuffer;
}

void SWGraphicsInterface::beginScene()
{
	Matrix4 defaultProjectionMatrix = Camera::buildMatrixOrtho2D(0, m_vResolution.x, m_vResolution.y, 0);

	// push main transforms
	pushTransform();
	setProjectionMatrix(defaultProjectionMatrix);
	translate(r_globaloffset_x->getFloat(), r_globaloffset_y->getFloat());

	// and apply them
	updateTransform();

	// clear backbuffer
	memset(m_backBuffer, 0, sizeof(PIXEL) * (int)(m_vResolution.x*m_vResolution.y));
}

void SWGraphicsInterface::endScene()
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

void SWGraphicsInterface::clearDepthBuffer()
{
	// TODO:
}

void SWGraphicsInterface::setColor(Color color)
{
	m_color = color;
}

void SWGraphicsInterface::setAlpha(float alpha)
{
	m_color &= 0x00ffffff;
	m_color |= ((int)(255.0f * alpha)) << 24;
	setColor(m_color);
}

void SWGraphicsInterface::drawPixels(int x, int y, int width, int height, Graphics::DRAWPIXELS_TYPE type, const void *pixels)
{
	// TODO:
	/*
	glRasterPos2i(x, y+height); // '+height' because of opengl bottom left origin, but engine top left origin
	glDrawPixels(width, height, GL_RGBA, (type == DRAWPIXELS_UBYTE ? GL_UNSIGNED_BYTE : GL_FLOAT), pixels);
	*/
}

void SWGraphicsInterface::drawPixel(int x, int y)
{
	updateTransform();

	if (x < 0 || x > m_vResolution.x-1 || y < 0 || y > m_vResolution.y-1)
		return;

	const int index = x + (int)(y*m_vResolution.x);

	PIXEL ssrcPixel = getColorPixel(m_color);
	PIXEL sdstPixel = m_backBuffer[index];
	Color srcPixel = COLOR(ssrcPixel.a, ssrcPixel.r, ssrcPixel.g, ssrcPixel.b);
	Color dstPixel = COLOR(sdstPixel.a, sdstPixel.r, sdstPixel.g, sdstPixel.b);
	Color finalColor = COLORf((COLOR_GET_Af(srcPixel)*COLOR_GET_Af(srcPixel) + COLOR_GET_Af(dstPixel)*(1.0f - COLOR_GET_Af(srcPixel))),
							  (COLOR_GET_Rf(srcPixel)*COLOR_GET_Af(srcPixel) + COLOR_GET_Rf(dstPixel)*(1.0f - COLOR_GET_Af(srcPixel))),
							  (COLOR_GET_Gf(srcPixel)*COLOR_GET_Af(srcPixel) + COLOR_GET_Gf(dstPixel)*(1.0f - COLOR_GET_Af(srcPixel))),
							  (COLOR_GET_Bf(srcPixel)*COLOR_GET_Af(srcPixel) + COLOR_GET_Bf(dstPixel)*(1.0f - COLOR_GET_Af(srcPixel))));

	m_backBuffer[index] = getColorPixel(finalColor);
}

void SWGraphicsInterface::drawLine(int x1, int y1, int x2, int y2)
{
	// Bresenham's line algorithm
	const bool steep = (std::abs(y2 - y1) > std::abs(x2 - x1));
	if (steep)
	{
		std::swap(x1, y1);
		std::swap(x2, y2);
	}

	if (x1 > x2)
	{
		std::swap(x1, x2);
		std::swap(y1, y2);
	}

	const float dx = x2 - x1;
	const float dy = std::abs(y2 - y1);

	float error = dx / 2.0f;
	const int ystep = (y1 < y2) ? 1 : -1;
	int y = y1;

	const int maxX = x2;

	for (int x=x1; x<maxX; x++)
	{
		if (steep)
			drawPixel(y, x); // implicitly calls updateTransform()
		else
			drawPixel(x, y);

		error -= dy;
		if (error < 0)
		{
			y += ystep;
			error += dx;
		}
	}
}

void SWGraphicsInterface::drawLine(Vector2 pos1, Vector2 pos2)
{
	drawLine(pos1.x, pos1.y, pos2.x, pos2.y);
}

void SWGraphicsInterface::drawRect(int x, int y, int width, int height)
{
	drawLine(x, y, x+width, y);
	drawLine(x, y, x, y+height);
	drawLine(x, y+height, x+width+1, y+height);
	drawLine(x+width, y, x+width, y+height);
}

void SWGraphicsInterface::drawRect(int x, int y, int width, int height, Color top, Color right, Color bottom, Color left)
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

void SWGraphicsInterface::fillRect(int x, int y, int width, int height)
{
	updateTransform();

	// TODO: make this faster
	for (int xx=0; xx<width; xx++)
	{
		for (int yy=0; yy<height; yy++)
		{
			drawPixel(x + xx, y + yy);
		}
	}
}

void SWGraphicsInterface::fillRoundedRect(int x, int y, int width, int height, int radius)
{
	float xOffset= x + radius;
	float yOffset = y + radius;

	double i = 0;
	double factor = 0.05;

	updateTransform();

	// TODO:
	/*
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
	*/
}

void SWGraphicsInterface::fillGradient(int x, int y, int width, int height, Color topLeftColor, Color topRightColor, Color bottomLeftColor, Color bottomRightColor)
{
	updateTransform();
	// TODO:
}

void SWGraphicsInterface::drawQuad(int x, int y, int width, int height)
{
	updateTransform();
	// TODO:
}

void SWGraphicsInterface::drawQuad(Vector2 topLeft, Vector2 topRight, Vector2 bottomRight, Vector2 bottomLeft, Color topLeftColor, Color topRightColor, Color bottomRightColor, Color bottomLeftColor)
{
	updateTransform();
	// TODO:
}

void SWGraphicsInterface::drawImage(Image *image)
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

	// TODO:
	Vector4 start = m_worldMatrix * Vector4(x,y,0,1);
	Vector4 size = (m_worldMatrix * Vector4(x+width,y+height,0,1)) - start;
	drawRect(start.x, start.y, size.x, size.y);

	image->unbind();

	/*
	if (r_debug_drawimage.getBool())
	{
		setColor(0xbbff00ff);
		drawRect(x, y, width, height);
	}
	*/
}

void SWGraphicsInterface::drawString(McFont *font, UString text)
{
	if (font == NULL || text.length() < 1 || !font->isReady())
		return;

	updateTransform();

	font->drawString(this, text);
}

void SWGraphicsInterface::drawVAO(VertexArrayObject *vao)
{
	const std::vector<Vector3> &vertices = vao->getVertices();
	const std::vector<Vector3> &normals = vao->getNormals();
	const std::vector<std::vector<Vector2>> &texcoords = vao->getTexcoords();
	const std::vector<Color> &colors = vao->getColors();

	updateTransform();

	// TODO:
	/*
	GLenum type = GL_TRIANGLES;
	if (vao->getPrimitive() == Graphics::PRIMITIVE::PRIMITIVE_TRIANGLE_FAN)
		type = GL_TRIANGLE_FAN;
	else if (vao->getPrimitive() == Graphics::PRIMITIVE::PRIMITIVE_QUADS)
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
	*/
}

void SWGraphicsInterface::drawVB(VertexBuffer *vb)
{
	updateTransform();

	///vb->draw(this);
}

void SWGraphicsInterface::pushTransform()
{
	m_worldTransformStack.push(Matrix4(m_worldTransformStack.top()));
	m_projectionTransformStack.push(Matrix4(m_projectionTransformStack.top()));
}

void SWGraphicsInterface::popTransform()
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

void SWGraphicsInterface::translate(float x, float y, float z)
{
	m_worldTransformStack.top().translate(x, y, z);
	m_bTransformUpToDate = false;
}

void SWGraphicsInterface::rotate(float deg, float x, float y, float z)
{
	m_worldTransformStack.top().rotate(deg, x, y, z);
	m_bTransformUpToDate = false;
}

void SWGraphicsInterface::scale(float x, float y, float z)
{
	m_worldTransformStack.top().scale(x, y, z);
	m_bTransformUpToDate = false;
}

void SWGraphicsInterface::setWorldMatrix(Matrix4 &worldMatrix)
{
	m_worldTransformStack.pop();
	m_worldTransformStack.push(worldMatrix);
	m_bTransformUpToDate = false;
}

void SWGraphicsInterface::setWorldMatrixMul(Matrix4 &worldMatrix)
{
	Matrix4 newWorldMatrix = m_worldTransformStack.top() * worldMatrix;
	m_worldTransformStack.pop();
	m_worldTransformStack.push(newWorldMatrix);
	m_bTransformUpToDate = false;
}

void SWGraphicsInterface::setProjectionMatrix(Matrix4 &projectionMatrix)
{
	m_projectionTransformStack.pop();
	m_projectionTransformStack.push(projectionMatrix);
	m_bTransformUpToDate = false;
}

Matrix4 SWGraphicsInterface::getWorldMatrix()
{
	return m_worldMatrix;
}

Matrix4 SWGraphicsInterface::getProjectionMatrix()
{
	return m_projectionMatrix;
}

void SWGraphicsInterface::setClipRect(Rect clipRect)
{
	///if (r_debug_disable_cliprect.getBool()) return;
	//if (m_bIs3DScene) return;

	// TODO:
	/*
	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	//debugLog("viewport = %i, %i, %i, %i\n", viewport[0], viewport[1], viewport[2], viewport[3]);

	glEnable(GL_SCISSOR_TEST);
	glScissor((int)clipRect.getX()+viewport[0], viewport[3]-((int)clipRect.getY()-viewport[1]-1+(int)clipRect.getHeight()), (int)clipRect.getWidth(), (int)clipRect.getHeight());

	//debugLog("scissor = %i, %i, %i, %i\n", (int)clipRect.getX()+viewport[0], viewport[3]-((int)clipRect.getY()-viewport[1]-1+(int)clipRect.getHeight()), (int)clipRect.getWidth(), (int)clipRect.getHeight());
	*/
}

void SWGraphicsInterface::pushClipRect(Rect clipRect)
{
	if (m_clipRectStack.size() > 0)
		m_clipRectStack.push(m_clipRectStack.top().intersect(clipRect));
	else
		m_clipRectStack.push(clipRect);

	setClipRect(m_clipRectStack.top());
}

void SWGraphicsInterface::popClipRect()
{
	m_clipRectStack.pop();

	if (m_clipRectStack.size() > 0)
		setClipRect(m_clipRectStack.top());
	else
		setClipping(false);
}

void SWGraphicsInterface::pushStencil()
{
	// TODO:
}

void SWGraphicsInterface::fillStencil(bool inside)
{
	// TODO:
}

void SWGraphicsInterface::popStencil()
{
	// TODO:
}

void SWGraphicsInterface::push3DScene(Rect region)
{
	///if (r_debug_disable_3dscene.getBool()) return;

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

void SWGraphicsInterface::pop3DScene()
{
	if (!m_3dSceneStack.top())
		return;

	m_3dSceneStack.pop();

	// restore transforms
	popTransform();

	m_bIs3dScene = false;
}

void SWGraphicsInterface::translate3DScene(float x, float y, float z)
{
	// block if we're not in a 3d scene
	if (!m_3dSceneStack.top())
		return;

	// translate directly
	m_3dSceneWorldMatrix.translate(x,y,z);

	m_bTransformUpToDate = false;
	updateTransform();
}

void SWGraphicsInterface::rotate3DScene(float rotx, float roty, float rotz)
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

void SWGraphicsInterface::offset3DScene(float x, float y, float z)
{
	m_v3dSceneOffset = Vector3(x,y,z);
}

void SWGraphicsInterface::setClipping(bool enabled)
{
	// TODO:
	/*
	if (enabled)
	{
		if (m_clipRectStack.size() > 0)
			glEnable(GL_SCISSOR_TEST);
	}
	else
		glDisable(GL_SCISSOR_TEST);
	*/
}

void SWGraphicsInterface::setBlending(bool enabled)
{
	// TODO:
}

void SWGraphicsInterface::setDepthBuffer(bool enabled)
{
	// TODO:
}

void SWGraphicsInterface::setCulling(bool culling)
{
	// TODO:
}

void SWGraphicsInterface::setAntialiasing(bool aa)
{
	m_bAntiAliasing = aa;
}

void SWGraphicsInterface::setWireframe(bool enabled)
{
	// TODO:
}

std::vector<unsigned char> SWGraphicsInterface::getScreenshot()
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

	// TODO:

	// copy to vector
	result.reserve(numElements);
	result.assign(pixels, pixels + numElements);
	delete[] pixels;

	return result;
}

UString SWGraphicsInterface::getVendor()
{
	return "McEngine";
}

UString SWGraphicsInterface::getModel()
{
	return "SW Rasterizer";
}

UString SWGraphicsInterface::getVersion()
{
	return "1.0";
}

int SWGraphicsInterface::getVRAMTotal()
{
	return -1;
}

int SWGraphicsInterface::getVRAMRemaining()
{
	return -1;
}

void SWGraphicsInterface::onResolutionChange(Vector2 newResolution)
{
	m_vResolution = newResolution;

	// rebuild viewport
	if (m_backBuffer != NULL)
		delete[] m_backBuffer;
	m_backBuffer = new PIXEL[(int)(m_vResolution.x*m_vResolution.y)];
}

Image *SWGraphicsInterface::createImage(UString filePath, bool mipmapped)
{
	return new SWImage(filePath, mipmapped);
}

Image *SWGraphicsInterface::createImage(int width, int height, bool clampToEdge)
{
	return new SWImage(width, height, clampToEdge);
}

RenderTarget *SWGraphicsInterface::createRenderTarget(int x, int y, int width, int height, Graphics::MULTISAMPLE_TYPE multiSampleType)
{
	return new SWRenderTarget(x, y, width, height, multiSampleType);
}

Shader *SWGraphicsInterface::createShaderFromFile(UString vertexShaderFilePath, UString fragmentShaderFilePath)
{
	return new SWShader(vertexShaderFilePath, fragmentShaderFilePath, false);
}

Shader *SWGraphicsInterface::createShaderFromSource(UString vertexShader, UString fragmentShader)
{
	return new SWShader(vertexShader, fragmentShader, true);
}

void SWGraphicsInterface::updateTransform()
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

		m_projectionMatrix = projectionMatrixTemp;
		m_worldMatrix = worldMatrixTemp;

		m_bTransformUpToDate = true;
	}
}

SWGraphicsInterface::PIXEL SWGraphicsInterface::getColorPixel(const Color &color)
{
	PIXEL p;
	p.r = (unsigned char)COLOR_GET_Ri(color);
	p.g = (unsigned char)COLOR_GET_Gi(color);
	p.b = (unsigned char)COLOR_GET_Bi(color);
	p.a = (unsigned char)COLOR_GET_Ai(color);
	return p;
}
