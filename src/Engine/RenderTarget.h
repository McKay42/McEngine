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
	RenderTarget(int x, int y, int width, int height);
	~RenderTarget();

	void enable();
	void disable();

	void draw(Graphics *g, int x, int y);
	void draw(Graphics *g, int x, int y, int width, int height);
	void drawRect(Graphics *g, int x, int y, int width, int height);

	void bind(unsigned int textureUnit = 0);
	void unbind();

	void rebuild(int x, int y, int width, int height);

	// set
	void setPos(int x, int y) {m_vPos.x = x; m_vPos.y = y;}
	void setPos(Vector2 pos) {m_vPos = pos;}
	void setColor(Color color) {m_color = color;}
	void setClearColorOnDraw(bool clearColorOnDraw) {m_bClearColorOnDraw = clearColorOnDraw;}
	void setClearDepthOnDraw(bool clearDepthOnDraw) {m_bClearDepthOnDraw = clearDepthOnDraw;}

	// get
	float getWidth() {return m_vSize.x;}
	float getHeight() {return m_vSize.y;}
	inline Vector2 getPos() const {return m_vPos;}
	inline int getTexture() const {return m_iRenderTexture;}

private:
	void release();
	void build();

	bool m_bReady;
	bool m_bClearColorOnDraw;
	bool m_bClearDepthOnDraw;
	Vector2 m_vPos,m_vSize;

	Color m_color;

	unsigned int m_iFrameBuffer;
	unsigned int m_iRenderTexture;
	unsigned int m_iDepthBuffer;

	float m_projectionMatrixBackup[16];
	int m_iFrameBufferBackup;
};

#endif
