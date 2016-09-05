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
	CBaseUIImage(UString imageResourceName, float xPos, float yPos, float xSize, float ySize, UString name);
	virtual ~CBaseUIImage(){;}

	virtual void draw(Graphics *g);

	void setImage(Image *img);

	void setDrawFrame(bool drawFrame){m_bDrawFrame = drawFrame;}
	void setDrawBackground(bool drawBackground){m_bDrawBackground = drawBackground;}

	void setFrameColor(Color frameColor){m_frameColor = frameColor;}
	void setColor(Color color){m_color = color;}
	void setAlpha(float alpha){m_color &= 0x00ffffff; m_color |= ((int)(255.0f * alpha)) << 24;}
	void setBackgroundColor(Color backgroundColor){m_backgroundColor = backgroundColor;}

	void setRotationDeg(float rotation){m_fRot = rotation;}
	void setScale(float xScale, float yScale){m_vScale.x = xScale;m_vScale.y = yScale;}
	void setScale(Vector2 scale){m_vScale.x = scale.x; m_vScale.y = scale.y;}
	void setScaleToFit(bool scaleToFit) {m_bScaleToFit = scaleToFit;}

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
