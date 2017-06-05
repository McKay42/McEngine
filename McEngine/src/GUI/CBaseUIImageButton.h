//================ Copyright (c) 2015, PG, All rights reserved. =================//
//
// Purpose:		a simple image button
//
// $NoKeywords: $
//===============================================================================//

#ifndef CBASEUIIMAGEBUTTON_H
#define CBASEUIIMAGEBUTTON_H

#include "CBaseUIButton.h"

class CBaseUIImageButton : public CBaseUIButton
{
public:
	CBaseUIImageButton(UString imageResourceName="", float xPos=0, float yPos=0, float xSize=0, float ySize=0, UString name="");
	virtual ~CBaseUIImageButton() {;}

	ELEMENT_BODY(CBaseUIImageButton)

	virtual void draw(Graphics *g);

	virtual void onResized();

	CBaseUIImageButton *setImageResourceName(UString imageResourceName);
	CBaseUIImageButton *setRotationDeg(float deg) {m_fRot = deg; return this;}
	CBaseUIImageButton *setScale(float xScale, float yScale) {m_vScale.x = xScale; m_vScale.y = yScale; return this;}
	CBaseUIImageButton *setScaleToFit(bool scaleToFit) {m_bScaleToFit = scaleToFit; return this;}
	CBaseUIImageButton *setKeepAspectRatio(bool keepAspectRatio) {m_bKeepAspectRatio = keepAspectRatio; return this;}

	inline UString getImageResourceName() const {return m_sImageResourceName;}
	inline Vector2 getScale() const {return m_vScale;}

protected:
	UString m_sImageResourceName;

	float m_fRot;
	Vector2 m_vScale;
	bool m_bScaleToFit;
	bool m_bKeepAspectRatio;
};

#endif
