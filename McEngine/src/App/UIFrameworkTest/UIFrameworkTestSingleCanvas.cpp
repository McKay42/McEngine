/*
 * UIFrameworkTestSingleCanvas.cpp
 *
 *  Created on: May 29, 2017
 *      Author: Psy
 */

#include "UIFrameworkTestSingleCanvas.h"

#include "Engine.h"
#include "UIFrameworkTest.h"

#include "CBaseUIButton.h"
#include "CBaseUICanvas.h"
#include "CBaseUITextbox.h"

UIFrameworkTestSingleCanvas::UIFrameworkTestSingleCanvas(UIFrameworkTest *app) : UIFrameworkTestScreen(app)
{
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

	m_canvasTest = new CBaseUICanvas(0, 0, engine->getScreenWidth(), engine->getScreenHeight(), "TestCanvas");
	m_canvasTest->addElement(m_canvasTestTL);
	m_canvasTest->addElement(m_canvasTestTR);
	m_canvasTest->addElement(m_canvasTestBL);
	m_canvasTest->addElement(m_canvasTestBR);
	m_canvasTest->addElement(m_canvasTestHeightOnly)->setScaleByHeightOnly(true);

	m_canvasResizeButton = new CBaseUIButton(300, 500, 200, 25, "CanvasTestButton", "Resize Canvas");
	m_canvasResizeButton->setClickCallback(fastdelegate::MakeDelegate(this, &UIFrameworkTestSingleCanvas::resizeCanvas));

	m_canvasMoveButton = new CBaseUIButton(300, 400, 200, 25, "CanvasMoveButton", "Move Canvas");
	m_canvasMoveButton->setClickCallback(fastdelegate::MakeDelegate(this, &UIFrameworkTestSingleCanvas::moveCanvas));

	m_bCanvasResized = false;
	m_bCanvasMoved = false;
}

UIFrameworkTestSingleCanvas::~UIFrameworkTestSingleCanvas()
{
	m_canvasTest->clear();

	SAFE_DELETE(m_canvasTest);
	SAFE_DELETE(m_canvasResizeButton);
	SAFE_DELETE(m_canvasMoveButton);
}

void UIFrameworkTestSingleCanvas::resizeCanvas()
{
	if (m_bCanvasResized)
	{
		m_canvasTest->setSize(engine->getScreenSize());
		m_bCanvasResized=false;
		debugLog("Canvas Test: Set Size (%f, %f)", engine->getScreenSize().x, engine->getScreenSize().y);
	}
	else
	{
		m_canvasTest->setSize(800, 600);
		m_bCanvasResized=true;
		debugLog("Canvas Test: Set Size (800, 600)");
	}
}

void UIFrameworkTestSingleCanvas::draw(Graphics *g)
{
	// Canvas
	m_canvasTest->draw(g);
	m_canvasTest->drawDebug(g);

	m_canvasResizeButton->draw(g);
	m_canvasMoveButton->draw(g);
}

void UIFrameworkTestSingleCanvas::update()
{
	// Canvas
	m_canvasTest->update();
	m_canvasResizeButton->update();
	m_canvasMoveButton->update();
}

void UIFrameworkTestSingleCanvas::onResolutionChanged(Vector2 newResolution)
{
	if (!m_bCanvasResized)
	{
		m_canvasTest->setSize(newResolution);
		debugLog("Canvas Test: Set Size (%f, %f)", newResolution.x, newResolution.y);
	}
}

void UIFrameworkTestSingleCanvas::moveCanvas()
{
	if (m_bCanvasMoved)
	{
		m_canvasTest->setPos(0, 0);
		m_bCanvasMoved=false;
		debugLog("Canvas Test: Set Position (0, 0)");
	}
	else
	{
		m_canvasTest->setPos(100, 100);
		m_bCanvasMoved=true;
		debugLog("Canvas Test: Set Position (100, 100)");
	}
}
