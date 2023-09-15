//================ Copyright (c) 2019, PG, All rights reserved. =================//
//
// Purpose:		raw opengl es 2.x graphics interface
//
// $NoKeywords: $gles2i
//===============================================================================//

#ifndef OPENGLES2INTERFACE_H
#define OPENGLES2INTERFACE_H

#include "cbase.h"
#include "NullGraphicsInterface.h"

#ifdef MCENGINE_FEATURE_OPENGLES

class OpenGLES2Shader;

class OpenGLES2Interface : public NullGraphicsInterface
{
public:
	OpenGLES2Interface();
	virtual ~OpenGLES2Interface();

	// scene
	virtual void beginScene();
	virtual void endScene();

	// depth buffer
	virtual void clearDepthBuffer();

	// color
	virtual void setColor(Color color);
	virtual void setAlpha(float alpha);

	// 2d primitive drawing
	virtual void drawLine(int x1, int y1, int x2, int y2);
	virtual void drawLine(Vector2 pos1, Vector2 pos2);
	virtual void drawRect(int x, int y, int width, int height);
	virtual void drawRect(int x, int y, int width, int height, Color top, Color right, Color bottom, Color left);

	// TODO

	virtual void fillRect(int x, int y, int width, int height);
	virtual void fillGradient(int x, int y, int width, int height, Color topLeftColor, Color topRightColor, Color bottomLeftColor, Color bottomRightColor);

	virtual void drawQuad(int x, int y, int width, int height);
	virtual void drawQuad(Vector2 topLeft, Vector2 topRight, Vector2 bottomRight, Vector2 bottomLeft, Color topLeftColor, Color topRightColor, Color bottomRightColor, Color bottomLeftColor);

	// 2d resource drawing
	virtual void drawImage(Image *image);
	virtual void drawString(McFont *font, UString text);

	// 3d type drawing
	void drawVAO(VertexArrayObject *vao);

	// DEPRECATED: 2d clipping
	virtual void setClipRect(McRect clipRect);
	virtual void pushClipRect(McRect clipRect);
	virtual void popClipRect();

	// TODO

	// renderer settings
	virtual void setClipping(bool enabled);
	virtual void setAlphaTesting(bool enabled);
	virtual void setAlphaTestFunc(COMPARE_FUNC alphaFunc, float ref);
	virtual void setBlending(bool enabled);
	virtual void setBlendMode(BLEND_MODE blendMode);
	virtual void setDepthBuffer(bool enabled);
	virtual void setCulling(bool culling);
	virtual void setWireframe(bool enabled);

	// TODO

	// renderer info
	virtual Vector2 getResolution() const {return m_vResolution;}
	virtual int getVRAMTotal();
	virtual int getVRAMRemaining();

	// callbacks
	virtual void onResolutionChange(Vector2 newResolution);

	// factory
	virtual Image *createImage(UString filePath, bool mipmapped, bool keepInSystemMemory);
	virtual Image *createImage(int width, int height, bool mipmapped, bool keepInSystemMemory);
	virtual RenderTarget *createRenderTarget(int x, int y, int width, int height, Graphics::MULTISAMPLE_TYPE multiSampleType);
	virtual Shader *createShaderFromFile(UString vertexShaderFilePath, UString fragmentShaderFilePath);
	virtual Shader *createShaderFromSource(UString vertexShader, UString fragmentShader);
	virtual VertexArrayObject *createVertexArrayObject(Graphics::PRIMITIVE primitive, Graphics::USAGE_TYPE usage, bool keepInSystemMemory);

	// ILLEGAL:
	void forceUpdateTransform();

	inline const int getShaderGenericAttribPosition() const {return m_iShaderTexturedGenericAttribPosition;}
	inline const int getShaderGenericAttribUV() const {return m_iShaderTexturedGenericAttribUV;}
	inline const int getShaderGenericAttribCol() const {return m_iShaderTexturedGenericAttribCol;}

	inline const int getVBOVertices() const {return m_iVBOVertices;}
	inline const int getVBOTexcoords() const {return m_iVBOTexcoords;}
	inline const int getVBOTexcolors() const {return m_iVBOTexcolors;}

	Matrix4 getMVP() const {return m_MP;}

protected:
	virtual void init();
	virtual void onTransformUpdate(Matrix4 &projectionMatrix, Matrix4 &worldMatrix);

private:
	void handleGLErrors();

	static int primitiveToOpenGL(Graphics::PRIMITIVE primitive);
	static int compareFuncToOpenGL(Graphics::COMPARE_FUNC compareFunc);

	// renderer
	bool m_bInScene;
	Vector2 m_vResolution;
	Matrix4 m_projectionMatrix;
	Matrix4 m_worldMatrix;
	Matrix4 m_MP;

	OpenGLES2Shader *m_shaderTexturedGeneric;
	int m_iShaderTexturedGenericPrevType;
	int m_iShaderTexturedGenericAttribPosition;
	int m_iShaderTexturedGenericAttribUV;
	int m_iShaderTexturedGenericAttribCol;
	unsigned int m_iVBOVertices;
	unsigned int m_iVBOTexcoords;
	unsigned int m_iVBOTexcolors;

	// persistent vars
	Color m_color;

	// clipping
	std::stack<McRect> m_clipRectStack;
};

#endif

#endif
