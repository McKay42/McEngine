//================ Copyright (c) 2013, PG, All rights reserved. =================//
//
// Purpose:		offscreen rendering
//
// $NoKeywords: $rt
//===============================================================================//

#ifndef RENDERTARGET_H
#define RENDERTARGET_H

#include "cbase.h"

class RenderTarget
{
public:
	RenderTarget(int width, int height, Graphics::MULTISAMPLE_TYPE multiSampleType = Graphics::MULTISAMPLE_TYPE::MULTISAMPLE_0X) : RenderTarget(0, 0, width, height, multiSampleType) {;}
	RenderTarget(int x, int y, int width, int height, Graphics::MULTISAMPLE_TYPE multiSampleType = Graphics::MULTISAMPLE_TYPE::MULTISAMPLE_0X);
	~RenderTarget();

	void draw(Graphics *g, int x, int y);
	void draw(Graphics *g, int x, int y, int width, int height);
	void drawRect(Graphics *g, int x, int y, int width, int height);

	void enable();
	void disable();

	void bind(unsigned int textureUnit = 0);
	void unbind();

	void rebuild(int width, int height);
	void rebuild(int x, int y, int width, int height);

	// set
	void setPos(int x, int y) {m_vPos.x = x; m_vPos.y = y;}
	void setPos(Vector2 pos) {m_vPos = pos;}
	void setColor(Color color) {m_color = color;}
	void setClearColor(Color clearColor) {m_clearColor = clearColor;}
	void setClearColorOnDraw(bool clearColorOnDraw) {m_bClearColorOnDraw = clearColorOnDraw;}
	void setClearDepthOnDraw(bool clearDepthOnDraw) {m_bClearDepthOnDraw = clearDepthOnDraw;}

	// get
	float getWidth() {return m_vSize.x;}
	float getHeight() {return m_vSize.y;}
	inline Vector2 getSize() const {return m_vSize;}
	inline Vector2 getPos() const {return m_vPos;}
	inline Graphics::MULTISAMPLE_TYPE getMultiSampleType() const {return m_multiSampleType;}
	inline unsigned int getFrameBuffer() const {return m_iFrameBuffer;}
	inline unsigned int getRenderTexture() const {return m_iRenderTexture;}
	inline unsigned int getResolveFrameBuffer() const {return m_iResolveFrameBuffer;}
	inline unsigned int getResolveTexture() const {return m_iResolveTexture;}

	inline bool isMultiSampled() {return m_multiSampleType != Graphics::MULTISAMPLE_TYPE::MULTISAMPLE_0X;}

private:
	void release();
	void build();

	bool m_bReady;
	bool m_bClearColorOnDraw;
	bool m_bClearDepthOnDraw;

	Vector2 m_vPos,m_vSize;

	Graphics::MULTISAMPLE_TYPE m_multiSampleType;

	Color m_color;
	Color m_clearColor;

	unsigned int m_iFrameBuffer;
	unsigned int m_iRenderTexture;
	unsigned int m_iDepthBuffer;
	unsigned int m_iResolveFrameBuffer;
	unsigned int m_iResolveTexture;

	int m_iFrameBufferBackup;
	Vector2 m_vGraphicsResolutionBackup;
};

#endif
