//================ Copyright (c) 2012, PG, All rights reserved. =================//
//
// Purpose:		top level graphics interface
//
// $NoKeywords: $graphics
//===============================================================================//

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <vector>

class UString;
class Vector2;
class Matrix4;
class Image;
class Rect;

class McFont;
class Image;

class VertexArrayObject;
class VertexBuffer;

typedef unsigned long Color;

class Graphics
{
public:
	Graphics() {}
	virtual ~Graphics() {}

	enum DRAWPIXELS_TYPE
	{
		DRAWPIXELS_UBYTE,
		DRAWPIXELS_FLOAT
	};

	enum MULTISAMPLE_TYPE
	{
		MULTISAMPLE_0X,
		MULTISAMPLE_2X,
		MULTISAMPLE_4X,
		MULTISAMPLE_8X,
		MULTISAMPLE_16X
	};

	// scene
	virtual void beginScene() = 0;
	virtual void endScene() = 0;

	// depth buffer
	virtual void clearDepthBuffer() = 0;

	// color
	virtual void setColor(Color color) = 0;
	virtual void setAlpha(float alpha) = 0;

	// 2d primitive drawing
	virtual void drawPixels(int x, int y, int width, int height, DRAWPIXELS_TYPE type, const void *pixels) = 0;
	virtual void drawPixel(int x, int y) = 0;
	virtual void drawLine(int x1, int y1, int x2, int y2) = 0;
	virtual void drawLine(Vector2 pos1, Vector2 pos2) = 0;
	virtual void drawRect(int x, int y, int width, int height) = 0;
	virtual void drawRect(int x, int y, int width, int height, Color top, Color right, Color bottom, Color left) = 0;

	virtual void fillRect(int x, int y, int width, int height) = 0;
	virtual void fillRoundedRect(int x, int y, int width, int height, int radius) = 0;
	virtual void fillGradient(int x, int y, int width, int height, Color topLeftColor, Color topRightColor, Color bottomLeftColor, Color bottomRightColor) = 0;

	virtual void drawQuad(int x, int y, int width, int height) = 0;
	virtual void drawQuad(Vector2 topLeft, Vector2 topRight, Vector2 bottomRight, Vector2 bottomLeft, Color topLeftColor, Color topRightColor, Color bottomRightColor, Color bottomLeftColor) = 0;

	// 2d resource drawing
	virtual void drawImage(Image *image) = 0;
	virtual void drawString(McFont *font, UString text) = 0;

	// 3d type drawing
	virtual void drawVAO(VertexArrayObject *vao) = 0;
	virtual void drawVB(VertexBuffer *vb) = 0;

	// matrices & transforms
	virtual void pushTransform() = 0;
	virtual void popTransform() = 0;

	virtual void translate(float x, float y, float z = 0) = 0;
	virtual void rotate(float deg, float x = 0, float y = 0, float z = 1) = 0;
	virtual void scale(float x, float y, float z = 1) = 0;

	virtual void setWorldMatrix(Matrix4 &worldMatrix) = 0;
	virtual void setWorldMatrixMul(Matrix4 &worldMatrix) = 0;
	virtual void setProjectionMatrix(Matrix4 &projectionMatrix) = 0;

	virtual Matrix4 getWorldMatrix() = 0;
	virtual Matrix4 getProjectionMatrix() = 0;

	// DEPRECATED: 2d clipping
	virtual void setClipRect(Rect clipRect) = 0;
	virtual void pushClipRect(Rect clipRect) = 0;
	virtual void popClipRect() = 0;

	// stencil buffer
	virtual void pushStencil() = 0;
	virtual void fillStencil(bool inside) = 0;
	virtual void popStencil() = 0;

	// 3d gui scenes
	virtual void push3DScene(Rect region) = 0;
	virtual void pop3DScene() = 0;
	virtual void translate3DScene(float x, float y, float z) = 0;
	virtual void rotate3DScene(float rotx, float roty, float rotz) = 0;
	virtual void offset3DScene(float x, float y, float z) = 0;

	// renderer settings
	virtual void setClipping(bool enabled) = 0;
	virtual void setBlending(bool enabled) = 0;
	virtual void setDepthBuffer(bool enabled) = 0;
	virtual void setCulling(bool enabled) = 0;
	virtual void setVSync(bool enabled) = 0;
	virtual void setAntialiasing(bool enabled) = 0;

	// renderer actions
	virtual std::vector<unsigned char> getScreenshot() = 0;

	// renderer info
	virtual Vector2 getResolution() const = 0;
	virtual UString getVendor() = 0;
	virtual UString getModel() = 0;
	virtual UString getVersion() = 0;
	virtual int getVRAMTotal() = 0;
	virtual int getVRAMRemaining() = 0;

	// callbacks
	virtual void onResolutionChange(Vector2 newResolution) = 0;

protected:
	virtual void init() = 0; // must be called after the OS implementation constructor
};

#endif
