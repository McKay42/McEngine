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
	virtual void onMouseInside() {setSize(m_vSizeNormal * 1.1);}
	virtual void onMouseOutside() {setSize(m_vSizeNormal);}
};

UIFrameworkTest::UIFrameworkTest()
{
	// Anchor
	m_anchorTestButton = new UIFrameworkAnchorTestButton(300, 600, 200, 25, "AnchorTestButton", "Anchor Test");
	m_anchorTestButton->setAnchor(0, 0);
	m_anchorTestButton->setClickCallback(fastdelegate::MakeDelegate(this, &UIFrameworkTest::changeTestAnchor));

	m_bAnchorChanged=false;

	// Canvas Container
	m_canvasTestTL = new CBaseUITextbox(0, 0, 0.2, 0.1, "CanvasTestTopLeft");
	m_canvasTestTL->setText("Top Left");
	m_canvasTestTL->setTextJustification(1);
	m_canvasTestTL->setAnchor(0, 0);

	m_canvasTestTR = new CBaseUITextbox(1, 0, 0.2, 0.1, "CanvasTestTopRight");
	m_canvasTestTR->setText("Top Right");
	m_canvasTestTR->setTextJustification(1);
	m_canvasTestTR->setAnchor(1, 0);

	m_canvasTestBL = new CBaseUITextbox(0, 1, 0.2, 0.1, "CanvasTestBottomLeft");
	m_canvasTestBL->setText("Bottom Left");
	m_canvasTestBL->setTextJustification(1);
	m_canvasTestBL->setAnchor(0, 1);

	m_canvasTestBR = new CBaseUITextbox(1, 1, 0.2, 0.1, "CanvasTestBottomRight");
	m_canvasTestBR->setText("Bottom Right");
	m_canvasTestBR->setTextJustification(1);
	m_canvasTestBR->setAnchor(1, 1);

	m_canvasTestHeightOnly = new CBaseUITextbox(0.5, 0.2, 0.1125, 0.2, "CanvasTestHeightOnly");
	m_canvasTestHeightOnly->setText("Height Only");
	m_canvasTestHeightOnly->setTextJustification(1);
	m_canvasTestHeightOnly->setAnchor(0.5, 0.5);

	m_canvasTest = new CBaseUICanvas(0, 0, engine->getScreenWidth(), engine->getScreenHeight());
	m_canvasTest->addSlot(m_canvasTestTL);
	m_canvasTest->addSlot(m_canvasTestTR);
	m_canvasTest->addSlot(m_canvasTestBL);
	m_canvasTest->addSlot(m_canvasTestBR);
	m_canvasTest->addSlot(m_canvasTestHeightOnly, true);

	m_canvasResizeButton = new CBaseUIButton(300, 500, 200, 25, "CanvasTestButton", "Resize Canvas");
	m_canvasResizeButton->setClickCallback(fastdelegate::MakeDelegate(this, &UIFrameworkTest::resizeCanvas));

	m_canvasMoveButton = new CBaseUIButton(300, 400, 200, 25, "CanvasMoveButton", "Move Canvas");
	m_canvasMoveButton->setClickCallback(fastdelegate::MakeDelegate(this, &UIFrameworkTest::moveCanvas));

	m_bCanvasResized=false;
	m_bCanvasMoved=false;
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
	m_canvasResizeButton->draw(g);
	m_canvasMoveButton->draw(g);
}

void UIFrameworkTest::update()
{
	// Anchor Test button
	m_anchorTestButton->update();

	// Canvas
	m_canvasTest->update();
	m_canvasResizeButton->update();
	m_canvasMoveButton->update();
}

void UIFrameworkTest::changeTestAnchor(){
	if (m_bAnchorChanged){
		m_anchorTestButton->setAnchor(0, 0);
		m_bAnchorChanged=false;
		debugLog("Anchor Test: Set Anchor (0, 0)");
	}

	else{
		m_anchorTestButton->setAnchor(0.5, 0.5);
		m_bAnchorChanged=true;
		debugLog("Anchor Test: Set Anchor (0.5, 0.5)");
	}
}

void UIFrameworkTest::resizeCanvas(){
	if (m_bCanvasResized){
		m_canvasTest->setSize(engine->getScreenSize());
		m_bCanvasResized=false;
		debugLog("Canvas Test: Set Size (%f, %f)", engine->getScreenWidth(), engine->getScreenHeight());
	}

	else{
		m_canvasTest->setSize(800, 600);
		m_bCanvasResized=true;
		debugLog("Canvas Test: Set Size (800, 600)");
	}
}

void UIFrameworkTest::moveCanvas(){
	if (m_bCanvasMoved){
		m_canvasTest->setPos(0, 0);
		m_bCanvasMoved=false;
		debugLog("Canvas Test: Set Position (0, 0)");
	}

	else{
		m_canvasTest->setPos(100, 100);
		m_bCanvasMoved=true;
		debugLog("Canvas Test: Set Position (100, 100)");
	}
}

