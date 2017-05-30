/*
 * UIFrameworktTestMultiCanvas.cpp
 *
 *  Created on: May 29, 2017
 *      Author: Psy
 */

#include "UIFrameworkTestMultiCanvas.h"
#include "UIFrameworkTest.h"
#include "CBaseUICanvas.h"
#include "CBaseUIButton.h"
#include "CBaseUITextbox.h"

UIFrameworkTestMultiCanvas::UIFrameworkTestMultiCanvas(UIFrameworkTest *app) : UIFrameworkTestScreen(app)
{

	// Main Canvas
	m_canvasMain = new CBaseUICanvas(0, 0, engine->getScreenWidth(), engine->getScreenHeight(), "CanvasMain");

	// Inner Canvas
	m_canvasTL = new CBaseUICanvas(0, 0, 0.5, 0.5, "CanvasTL");
	m_canvasTL->setAnchor(0, 0);

	m_canvasTR = new CBaseUICanvas(1, 0, 0.5, 0.5, "CanvasTR");
	m_canvasTR->setAnchor(1, 0);

	m_canvasBL = new CBaseUICanvas(0, 1, 0.5, 0.5, "CanvasBL");
	m_canvasBL->setAnchor(0, 1);

	m_canvasBR = new CBaseUICanvas(1, 1, 0.5, 0.5, "CanvasBR");
	m_canvasBR->setAnchor(1, 1);

	// Canvas Helpers
	// Top Left
	m_canvasHelperTL_TL = new CBaseUITextbox(0, 0, 0.4, 0.4, "HelperTL");
	m_canvasHelperTL_TL->setText("Top Left");
	m_canvasHelperTL_TL->setTextJustification(1);
	m_canvasHelperTL_TL->setAnchor(0, 0);

	m_canvasHelperTL_TR = new CBaseUITextbox(1, 0, 0.4, 0.4, "HelperTR");
	m_canvasHelperTL_TR->setText("Top Right");
	m_canvasHelperTL_TR->setTextJustification(1);
	m_canvasHelperTL_TR->setAnchor(1, 0);

	m_canvasHelperTL_BL = new CBaseUITextbox(0, 1, 0.4, 0.4, "HelperBL");
	m_canvasHelperTL_BL->setText("Bottom Left");
	m_canvasHelperTL_BL->setTextJustification(1);
	m_canvasHelperTL_BL->setAnchor(0, 1);

	m_canvasHelperTL_BR = new CBaseUITextbox(1, 1, 0.4, 0.4, "HelperBR");
	m_canvasHelperTL_BR->setText("Bottom Right");
	m_canvasHelperTL_BR->setTextJustification(1);
	m_canvasHelperTL_BR->setAnchor(1, 1);

	// Top Right
	m_canvasHelperTR_TL = new CBaseUITextbox(0, 0, 0.4, 0.4, "HelperTL");
	m_canvasHelperTR_TL->setText("Top Left");
	m_canvasHelperTR_TL->setTextJustification(1);
	m_canvasHelperTR_TL->setAnchor(0, 0);

	m_canvasHelperTR_TR = new CBaseUITextbox(1, 0, 0.4, 0.4, "HelperTR");
	m_canvasHelperTR_TR->setText("Top Right");
	m_canvasHelperTR_TR->setTextJustification(1);
	m_canvasHelperTR_TR->setAnchor(1, 0);

	m_canvasHelperTR_BL = new CBaseUITextbox(0, 1, 0.4, 0.4, "HelperBL");
	m_canvasHelperTR_BL->setText("Bottom Left");
	m_canvasHelperTR_BL->setTextJustification(1);
	m_canvasHelperTR_BL->setAnchor(0, 1);

	m_canvasHelperTR_BR = new CBaseUITextbox(1, 1, 0.4, 0.4, "HelperBR");
	m_canvasHelperTR_BR->setText("Bottom Right");
	m_canvasHelperTR_BR->setTextJustification(1);
	m_canvasHelperTR_BR->setAnchor(1, 1);

	// Bottom Left
	m_canvasHelperBL_TL = new CBaseUITextbox(0, 0, 0.4, 0.4, "HelperTL");
	m_canvasHelperBL_TL->setText("Top Left");
	m_canvasHelperBL_TL->setTextJustification(1);
	m_canvasHelperBL_TL->setAnchor(0, 0);

	m_canvasHelperBL_TR = new CBaseUITextbox(1, 0, 0.4, 0.4, "HelperTR");
	m_canvasHelperBL_TR->setText("Top Right");
	m_canvasHelperBL_TR->setTextJustification(1);
	m_canvasHelperBL_TR->setAnchor(1, 0);

	m_canvasHelperBL_BL = new CBaseUITextbox(0, 1, 0.4, 0.4, "HelperBL");
	m_canvasHelperBL_BL->setText("Bottom Left");
	m_canvasHelperBL_BL->setTextJustification(1);
	m_canvasHelperBL_BL->setAnchor(0, 1);

	m_canvasHelperBL_BR = new CBaseUITextbox(1, 1, 0.4, 0.4, "HelperBR");
	m_canvasHelperBL_BR->setText("Bottom Right");
	m_canvasHelperBL_BR->setTextJustification(1);
	m_canvasHelperBL_BR->setAnchor(1, 1);

	// Bottom Right
	m_canvasHelperBR_TL = new CBaseUITextbox(0, 0, 0.4, 0.4, "HelperTL");
	m_canvasHelperBR_TL->setText("Top Left");
	m_canvasHelperBR_TL->setTextJustification(1);
	m_canvasHelperBR_TL->setAnchor(0, 0);

	m_canvasHelperBR_TR = new CBaseUITextbox(1, 0, 0.4, 0.4, "HelperTR");
	m_canvasHelperBR_TR->setText("Top Right");
	m_canvasHelperBR_TR->setTextJustification(1);
	m_canvasHelperBR_TR->setAnchor(1, 0);

	m_canvasHelperBR_BL = new CBaseUITextbox(0, 1, 0.4, 0.4, "HelperBL");
	m_canvasHelperBR_BL->setText("Bottom Left");
	m_canvasHelperBR_BL->setTextJustification(1);
	m_canvasHelperBR_BL->setAnchor(0, 1);

	m_canvasHelperBR_BR = new CBaseUITextbox(1, 1, 0.4, 0.4, "HelperBR");
	m_canvasHelperBR_BR->setText("Bottom Right");
	m_canvasHelperBR_BR->setTextJustification(1);
	m_canvasHelperBR_BR->setAnchor(1, 1);

	// Buttons
	m_canvasMainResize = new CBaseUIButton(300, 600, 300, 25, "canvasMainResize", "Resize Main Canvas");
	m_canvasMainResize->setClickCallback(fastdelegate::MakeDelegate(this, &UIFrameworkTestMultiCanvas::canvasMainResize));

	m_canvasMainMove = new CBaseUIButton(300, 500, 300, 25, "canvasMainMove", "Move Main Canvas");
	m_canvasMainMove->setClickCallback(fastdelegate::MakeDelegate(this, &UIFrameworkTestMultiCanvas::canvasMainMove));

	m_canvasInnerResize = new CBaseUIButton(300, 400, 300, 25, "canvasInnerResize", "Resize Inner Canvas");
	m_canvasInnerResize->setClickCallback(fastdelegate::MakeDelegate(this, &UIFrameworkTestMultiCanvas::canvasInnerResize));

	// Set UI Hierarchy
	// Top Left Canvas Helpers
	m_canvasTL->addSlot(m_canvasHelperTL_TL);
	m_canvasTL->addSlot(m_canvasHelperTL_TR);
	m_canvasTL->addSlot(m_canvasHelperTL_BL);
	m_canvasTL->addSlot(m_canvasHelperTL_BR);

	// Top Right Canvas Helpers
	m_canvasTR->addSlot(m_canvasHelperTR_TL);
	m_canvasTR->addSlot(m_canvasHelperTR_TR);
	m_canvasTR->addSlot(m_canvasHelperTR_BL);
	m_canvasTR->addSlot(m_canvasHelperTR_BR);

	// Bottom Left Canvas Helpers
	m_canvasBL->addSlot(m_canvasHelperBL_TL);
	m_canvasBL->addSlot(m_canvasHelperBL_TR);
	m_canvasBL->addSlot(m_canvasHelperBL_BL);
	m_canvasBL->addSlot(m_canvasHelperBL_BR);

	// Bottom Right Canvas Helpers
	m_canvasBR->addSlot(m_canvasHelperBR_TL);
	m_canvasBR->addSlot(m_canvasHelperBR_TR);
	m_canvasBR->addSlot(m_canvasHelperBR_BL);
	m_canvasBR->addSlot(m_canvasHelperBR_BR);

	// Inner Canvas
	m_canvasMain->addSlot(m_canvasTL);
	m_canvasMain->addSlot(m_canvasTR);
	m_canvasMain->addSlot(m_canvasBL);
	m_canvasMain->addSlot(m_canvasBR);


	m_bCanvasMainResize=false;
	m_bCanvasMainMove=false;
	m_bCanvasInnerResize=false;
}

UIFrameworkTestMultiCanvas::~UIFrameworkTestMultiCanvas()
{
	m_canvasMain->clear();
	SAFE_DELETE(m_canvasMainResize);
	SAFE_DELETE(m_canvasMainMove);
	SAFE_DELETE(m_canvasInnerResize);
}

void UIFrameworkTestMultiCanvas::draw(Graphics *g)
{
	m_canvasMain->draw(g);
	m_canvasMainResize->draw(g);
	m_canvasMainMove->draw(g);
	m_canvasInnerResize->draw(g);

	m_canvasTL->drawDebug(g, COLOR(255, 0, 255, 0));
	m_canvasTR->drawDebug(g, COLOR(255, 0, 255, 0));
	m_canvasBL->drawDebug(g, COLOR(255, 0, 255, 0));
	m_canvasBR->drawDebug(g, COLOR(255, 0, 255, 0));
	m_canvasMain->drawDebug(g);
}

void UIFrameworkTestMultiCanvas::update()
{
	m_canvasMain->update();
	m_canvasMainResize->update();
	m_canvasMainMove->update();
	m_canvasInnerResize->update();
}

void UIFrameworkTestMultiCanvas::canvasMainResize()
{
	if (m_bCanvasMainResize)
	{
		m_canvasMain->setSize(engine->getScreenSize());
		m_bCanvasMainResize=false;
	}
	else
	{
		m_canvasMain->setSize(800, 600);
		m_bCanvasMainResize=true;
	}

}

void UIFrameworkTestMultiCanvas::canvasMainMove()
{
	if (m_bCanvasMainMove)
	{
		m_canvasMain->setPos(0, 0);
		m_bCanvasMainMove=false;
	}
	else
	{
		m_canvasMain->setPos(150, 150);
		m_bCanvasMainMove=true;
	}
}

void UIFrameworkTestMultiCanvas::canvasInnerResize()
{
	if (m_bCanvasInnerResize)
	{
		m_canvasTL->setRelSize(0.5, 0.5);
		m_canvasTR->setRelSize(0.5, 0.5);
		m_canvasBL->setRelSize(0.5, 0.5);
		m_canvasBR->setRelSize(0.5, 0.5);
		m_bCanvasInnerResize=false;
	}
	else
	{
		m_canvasTL->setRelSize(0.25, 0.25);
		m_canvasTR->setRelSize(0.75, 0.25);
		m_canvasBL->setRelSize(0.5, 0.75);
		m_canvasBR->setRelSize(0.5, 0.75);
		m_bCanvasInnerResize=true;
	}
}
