//================ Copyright (c) 2016, PG, All rights reserved. =================//
//
// Purpose:		software renderer, just for fun
//
// $NoKeywords: $swi
//===============================================================================//

#ifndef SWGRAPHICSINTERFACE_H
#define SWGRAPHICSINTERFACE_H

#include "Graphics.h"
#include "cbase.h"

class SWGraphicsInterface : public Graphics
{
public:
	struct PIXEL
	{
		unsigned char b,g,r,a;
	};

public:
	SWGraphicsInterface();
	virtual ~SWGraphicsInterface();

	// scene
	virtual void beginScene();
	virtual void endScene();

	// depth buffer
	virtual void clearDepthBuffer();

	// color
	virtual void setColor(Color color);
	virtual void setAlpha(float alpha);

	// 2d primitive drawing
	virtual void drawPixels(int x, int y, int width, int height, Graphics::DRAWPIXELS_TYPE type, const void *pixels);
	virtual void drawPixel(int x, int y);
	virtual void drawLine(int x1, int y1, int x2, int y2);
	virtual void drawLine(Vector2 pos1, Vector2 pos2);
	virtual void drawRect(int x, int y, int width, int height);
	virtual void drawRect(int x, int y, int width, int height, Color top, Color right, Color bottom, Color left);

	virtual void fillRect(int x, int y, int width, int height);
	virtual void fillRoundedRect(int x, int y, int width, int height, int radius);
	virtual void fillGradient(int x, int y, int width, int height, Color topLeftColor, Color topRightColor, Color bottomLeftColor, Color bottomRightColor);

	virtual void drawQuad(int x, int y, int width, int height);
	virtual void drawQuad(Vector2 topLeft, Vector2 topRight, Vector2 bottomRight, Vector2 bottomLeft, Color topLeftColor, Color topRightColor, Color bottomRightColor, Color bottomLeftColor);

	// 2d resource drawing
	virtual void drawImage(Image *image);
	virtual void drawString(McFont *font, UString text);

	// 3d type drawing
	virtual void drawVAO(VertexArrayObject *vao);
	virtual void drawVB(VertexBuffer *vb);

	// matrices & transforms
	virtual void pushTransform();
	virtual void popTransform();

	virtual void translate(float x, float y, float z = 0);
	virtual void rotate(float deg, float x, float y, float z = 1);
	virtual void scale(float x, float y, float z = 1);

	virtual void setWorldMatrix(Matrix4 &worldMatrix);
	virtual void setWorldMatrixMul(Matrix4 &worldMatrix);
	virtual void setProjectionMatrix(Matrix4 &projectionMatrix);

	virtual Matrix4 getWorldMatrix();
	virtual Matrix4 getProjectionMatrix();

	// DEPRECATED: 2d clipping
	virtual void setClipRect(Rect clipRect);
	virtual void pushClipRect(Rect clipRect);
	virtual void popClipRect();

	// stencil
	virtual void pushStencil();
	virtual void fillStencil(bool inside);
	virtual void popStencil();

	// 3d gui scenes
	virtual void push3DScene(Rect region);
	virtual void pop3DScene();
	virtual void translate3DScene(float x, float y, float z);
	virtual void rotate3DScene(float rotx, float roty, float rotz);
	virtual void offset3DScene(float x, float y, float z);

	// renderer settings
	virtual void setClipping(bool enabled);
	virtual void setBlending(bool enabled);
	virtual void setDepthBuffer(bool enabled);
	virtual void setCulling(bool culling);
	virtual void setAntialiasing(bool aa);
	virtual void setWireframe(bool enabled);

	// renderer actions
	virtual std::vector<unsigned char> getScreenshot();

	// renderer info
	virtual Vector2 getResolution() const {return m_vResolution;}
	virtual UString getVendor();
	virtual UString getModel();
	virtual UString getVersion();
	virtual int getVRAMTotal();
	virtual int getVRAMRemaining();

	// callbacks
	virtual void onResolutionChange(Vector2 newResolution);

	// factory
	virtual Image *createImage(UString filePath, bool mipmapped);
	virtual Image *createImage(int width, int height, bool clampToEdge);
	virtual RenderTarget *createRenderTarget(int x, int y, int width, int height, Graphics::MULTISAMPLE_TYPE multiSampleType);
	virtual Shader *createShaderFromFile(UString vertexShaderFilePath, UString fragmentShaderFilePath);
	virtual Shader *createShaderFromSource(UString vertexShader, UString fragmentShader);

protected:
	void init();

	inline PIXEL *getBackBuffer() const {return m_backBuffer;}

private:
	void updateTransform();
	PIXEL getColorPixel(const Color &color);

	// renderer
	Vector2 m_vResolution;
	PIXEL *m_backBuffer;

	// transforms
	bool m_bTransformUpToDate;
	std::stack<Matrix4> m_worldTransformStack;
	std::stack<Matrix4> m_projectionTransformStack;
	Matrix4 m_worldMatrix;
	Matrix4 m_projectionMatrix;

	// persistent vars
	bool m_bAntiAliasing;
	Color m_color;
	float m_fZ;
	float m_fClearZ;

	// clipping
	std::stack<Rect> m_clipRectStack;

	// 3d gui scenes
	bool m_bIs3dScene;
	std::stack<bool> m_3dSceneStack;
	Rect m_3dSceneRegion;
	Vector3 m_v3dSceneOffset;
	Matrix4 m_3dSceneWorldMatrix;
	Matrix4 m_3dSceneProjectionMatrix;
};

#endif
