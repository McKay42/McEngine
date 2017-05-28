/*
 * UIFrameworkTest.cpp
 *
 *  Created on: May 28, 2017
 *      Author: Psy
 */

#include <UIFrameworkTest/UIFrameworkTest.h>

#include "CBaseUIButton.h"

class UIFrameworkAnchorTestButton : public CBaseUIButton
{
public:
	Vector2 m_vSizeNormal;

	UIFrameworkAnchorTestButton(float xPos, float yPos, float xSize, float ySize, UString name, UString text) : CBaseUIButton(xPos, yPos, xSize, ySize, name, text) {m_vSizeNormal.x = xSize; m_vSizeNormal.y = ySize;}
	virtual void onMouseDownInside() {debugLog("Anchor Test: Set Anchor - {0, 0}"); setAnchor(0, 0);}
	virtual void onMouseDownOutside() {debugLog("Anchor Test: Set Anchor - {1, 1}"); setAnchor(1, 1);}
	virtual void onMouseInside() {setSize(m_vSizeNormal * 1.1);}
	virtual void onMouseOutside() {setSize(m_vSizeNormal);}
};

UIFrameworkTest::UIFrameworkTest()
{
	m_anchorTestButton = new UIFrameworkAnchorTestButton(300, 600, 200, 25, "AnchorTestButton", "Anchor Test");
	m_anchorTestButton->setAnchor(0.5, 0.5);
	debugLog("Anchor Test: Set Anchor - {0.5, 0.5}");
}

UIFrameworkTest::~UIFrameworkTest()
{
	SAFE_DELETE(m_anchorTestButton);
}

void UIFrameworkTest::draw(Graphics *g)
{
	// Anchor Test button
	m_anchorTestButton->draw(g);
}

void UIFrameworkTest::update()
{
	// Anchor Test button
	m_anchorTestButton->update();
}

