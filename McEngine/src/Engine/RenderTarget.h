//================ Copyright (c) 2013, PG, All rights reserved. =================//
//
// Purpose:		offscreen rendering
//
// $NoKeywords: $rt
//===============================================================================//

#ifndef RENDERTARGET_H
#define RENDERTARGET_H

#include "Resource.h"

class ConVar;

class RenderTarget : public Resource
{
public:
	RenderTarget(int x, int y, int width, int height, Graphics::MULTISAMPLE_TYPE multiSampleType = Graphics::MULTISAMPLE_TYPE::MULTISAMPLE_0X);
	virtual ~RenderTarget() {;}

	virtual void draw(Graphics *g, int x, int y);
	virtual void draw(Graphics *g, int x, int y, int width, int height);
	virtual void drawRect(Graphics *g, int x, int y, int width, int height);

	virtual void enable() = 0;
	virtual void disable() = 0;

	virtual void bind(unsigned int textureUnit = 0) = 0;
	virtual void unbind() = 0;

	void rebuild(int x, int y, int width, int height, Graphics::MULTISAMPLE_TYPE multiSampleType);
	void rebuild(int x, int y, int width, int height);
	void rebuild(int width, int height);
	void rebuild(int width, int height, Graphics::MULTISAMPLE_TYPE multiSampleType);

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

	inline bool isMultiSampled() {return m_multiSampleType != Graphics::MULTISAMPLE_TYPE::MULTISAMPLE_0X;}

protected:
	static ConVar *debug_rt;

	virtual void init() = 0;
	virtual void initAsync() = 0;
	virtual void destroy() = 0;

	bool m_bClearColorOnDraw;
	bool m_bClearDepthOnDraw;

	Vector2 m_vPos;
	Vector2 m_vSize;

	Graphics::MULTISAMPLE_TYPE m_multiSampleType;

	Color m_color;
	Color m_clearColor;
};

#endif
