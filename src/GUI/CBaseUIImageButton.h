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
	CBaseUIImageButton(UString imageResourceName, float xPos, float yPos, float xSize, float ySize, UString name);
	virtual ~CBaseUIImageButton() {;}

	virtual void draw(Graphics *g);

	virtual void onResized();

	void setImageResourceName(UString imageResourceName);
	void setRotationDeg(float deg) {m_fRot = deg;}
	void setScale(float xScale, float yScale) {m_vScale.x = xScale; m_vScale.y = yScale;}
	void setScaleToFit(bool scaleToFit) {m_bScaleToFit = scaleToFit;}
	void setKeepAspectRatio(bool keepAspectRatio) {m_bKeepAspectRatio = keepAspectRatio;}

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
