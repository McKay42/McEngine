/*
 * UIFrameworkTest.cpp
 *
 *  Created on: May 28, 2017
 *      Author: Psy
 */

#include <UIFrameworkTest/UIFrameworkTest.h>

#include "CBaseUIButton.h"
#include "CBaseUICanvas.h"
#include "CBaseUITextbox.h"

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
	// Anchor
	m_anchorTestButton = new UIFrameworkAnchorTestButton(300, 600, 200, 25, "AnchorTestButton", "Anchor Test");
	m_anchorTestButton->setAnchor(0.5, 0.5);
	debugLog("Anchor Test: Set Anchor - {0.5, 0.5}");

	// Canvas Container
	m_canvasTest = new CBaseUICanvas(engine->getScreenWidth(), engine->getScreenHeight());

	m_canvasTestTL = new CBaseUITextbox(0, 0, 0.2, 0.1, "CanvasTestTopLeft");
	m_canvasTestTL->setText("Top Left");
	m_canvasTestTL->setAnchor(0, 0);

	m_canvasTestTR = new CBaseUITextbox(1, 0, 0.2, 0.1, "CanvasTestTopRight");
	m_canvasTestTR->setText("Top Right");
	m_canvasTestTR->setAnchor(1, 0);

	debugLog("m_canvasTestTR - Position: (%f, %f) - Size: (%f, %f) - Anchor: (%f, %f", m_canvasTestTR->getPos().x, m_canvasTestTR->getPos().y, m_canvasTestTR->getSize().x, m_canvasTestTR->getSize().y, m_canvasTestTR->getAnchor().x, m_canvasTestTR->getAnchor().y);;

	m_canvasTestBL = new CBaseUITextbox(0, 1, 0.2, 0.1, "CanvasTestBottomLeft");
	m_canvasTestBL->setText("Bottom Left");
	m_canvasTestBL->setAnchor(0, 1);

	m_canvasTestBR = new CBaseUITextbox(1, 1, 0.2, 0.1, "CanvasTestBottomRight");
	m_canvasTestBR->setText("Bottom Right");
	m_canvasTestBR->setAnchor(1, 1);

	m_canvasTest->addSlot(m_canvasTestTL);
	m_canvasTest->addSlot(m_canvasTestTR);
	m_canvasTest->addSlot(m_canvasTestBL);
	m_canvasTest->addSlot(m_canvasTestBR);

	m_canvasTestButton = new CBaseUIButton(300, 500, 200, 25, "CanvasTestButton", "Resize Canvas");
	m_canvasTestButton->setClickCallback(fastdelegate::MakeDelegate(this, &UIFrameworkTest::resizeCanvas));

	m_bResized=false;
}

UIFrameworkTest::~UIFrameworkTest()
{
	SAFE_DELETE(m_anchorTestButton);

	m_canvasTest->clear();
	SAFE_DELETE(m_canvasTest);
}

void UIFrameworkTest::draw(Graphics *g)
{
	// Anchor Test button
	m_anchorTestButton->draw(g);

	// Canvas
	m_canvasTest->draw(g);
	m_canvasTestButton->draw(g);
}

void UIFrameworkTest::update()
{
	// Anchor Test button
	m_anchorTestButton->update();

	// Canvas
	m_canvasTest->update();
	m_canvasTestButton->update();
}

void UIFrameworkTest::resizeCanvas(){
	if (m_bResized){
		m_canvasTest->setSize(engine->getScreenSize());
		m_bResized=false;
		return;
	}

	else{
		m_canvasTest->setSize(800, 600);
		m_bResized=true;
		return;
	}
}

