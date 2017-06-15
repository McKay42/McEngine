//================ Copyright (c) 2014, PG, All rights reserved. =================//
//
// Purpose:		a simple image class
//
// $NoKeywords: $
//===============================================================================//

#ifndef CBASEUIIMAGE_H
#define CBASEUIIMAGE_H

#include "CBaseUIElement.h"

class CBaseUIImage : public CBaseUIElement
{
public:
	CBaseUIImage(UString imageResourceName="", float xPos=0, float yPos=0, float xSize=0, float ySize=0, UString name="");
	virtual ~CBaseUIImage(){;}

	ELEMENT_BODY(CBaseUIImage)

	virtual void draw(Graphics *g);

	void setImage(Image *img);

	CBaseUIImage *setDrawFrame(bool drawFrame){m_bDrawFrame = drawFrame; return this;}
	CBaseUIImage *setDrawBackground(bool drawBackground){m_bDrawBackground = drawBackground; return this;}

	CBaseUIImage *setFrameColor(Color frameColor){m_frameColor = frameColor; return this;}
	CBaseUIImage *setColor(Color color){m_color = color; return this;}
	CBaseUIImage *setAlpha(float alpha){m_color &= 0x00ffffff; m_color |= ((int)(255.0f * alpha)) << 24; return this;}
	CBaseUIImage *setBackgroundColor(Color backgroundColor){m_backgroundColor = backgroundColor; return this;}

	CBaseUIImage *setRotationDeg(float rotation){m_fRot = rotation; return this;}
	CBaseUIImage *setScale(float xScale, float yScale){m_vScale.x = xScale;m_vScale.y = yScale; return this;}
	CBaseUIImage *setScale(Vector2 scale){m_vScale.x = scale.x; m_vScale.y = scale.y; return this;}
	CBaseUIImage *setScaleToFit(bool scaleToFit) {m_bScaleToFit = scaleToFit; return this;}

	inline float getRotationDeg() const {return m_fRot;}
	inline Vector2 getScale() const {return m_vScale;}
	inline Image *getImage() const {return m_image;}

private:
	Image *m_image;

	Color m_frameColor;
	Color m_backgroundColor;
	Color m_color;

	bool m_bDrawFrame;
	bool m_bDrawBackground;
	bool m_bScaleToFit;

	float m_fRot;
	Vector2 m_vScale;
};

#endif
