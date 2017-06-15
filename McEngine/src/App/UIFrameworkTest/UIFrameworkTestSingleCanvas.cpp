/*
 * UIFrameworkTestSingleCanvas.cpp
 *
 *  Created on: May 29, 2017
 *      Author: Psy
 */

#include "UIFrameworkTestSingleCanvas.h"

#include "Engine.h"
#include "UIFrameworkTest.h"

#include "CBaseUI.h"

UIFrameworkTestSingleCanvas::UIFrameworkTestSingleCanvas(UIFrameworkTest *app) : UIFrameworkTestScreen(app)
{
	m_canvasTest = std::shared_ptr<UI::Canvas>((new UI::Canvas)
					->setSize(engine->getScreenSize())
					->setName("TestCanvas")

					->addElement((new UI::Textbox)
								 ->setRelSize(0.2, 0.1)
								 ->setName("CanvasTestTopLeft")
								 ->setText("Top Left")
								 ->setTextJustification(1)
					)

					->addElement((new UI::Textbox)
								 ->setRelPos(1, 0)
								 ->setRelSize(0.2, 0.1)
								 ->setAnchor(1, 0)
								 ->setName("CanvasTestTopRight")
								 ->setText("Top Right")
								 ->setTextJustification(1)
					)

					->addElement((new UI::Textbox)
								 ->setRelPos(0, 1)
								 ->setRelSize(0.2, 0.1)
								 ->setAnchor(0, 1)
								 ->setName("CanvasTestBottomLeft")
								 ->setText("Bottom Left")
								 ->setTextJustification(1)
					)

					->addElement((new UI::Textbox)
								 ->setRelPos(1, 1)
								 ->setRelSize(0.2, 0.1)
								 ->setAnchor(1, 1)
								 ->setName("CanvasTestBottomRight")
								 ->setText("Bottom Right")
								 ->setTextJustification(1)
					)

					->addElement((new UI::Textbox)
								 ->setRelPos(0.5, 0.2)
								 ->setRelSize(0.1125, 0.2)
								 ->setAnchor(0.5, 0.5)
								 ->setName("CanvasTestHeightOnly")
								 ->setText("Height Only")
								 ->setTextJustification(1)
								 ->setScaleByHeightOnly(true)
					)
	);

	m_canvasResizeButton = std::make_shared<UI::Button>(300, 500, 200, 25, "CanvasTestButton", "Resize Canvas");
	m_canvasResizeButton->setClickCallback(fastdelegate::MakeDelegate(this, &UIFrameworkTestSingleCanvas::resizeCanvas));

	m_canvasMoveButton = std::make_shared<UI::Button>(300, 400, 200, 25, "CanvasMoveButton", "Move Canvas");
	m_canvasMoveButton->setClickCallback(fastdelegate::MakeDelegate(this, &UIFrameworkTestSingleCanvas::moveCanvas));

	m_bCanvasResized = false;
	m_bCanvasMoved = false;
}

UIFrameworkTestSingleCanvas::~UIFrameworkTestSingleCanvas()
{

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
