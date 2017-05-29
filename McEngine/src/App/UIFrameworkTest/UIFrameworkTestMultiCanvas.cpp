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

UIFrameworkTestMultiCanvas::UIFrameworkTestMultiCanvas(UIFrameworkTest *app) : UIFrameworkTestScreen(app) {

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
	m_canvasHelperTL = new CBaseUITextbox(0, 0, 0.4, 0.4, "HelperTL");
	m_canvasHelperTL->setText("Top Left");
	m_canvasHelperTL->setTextJustification(1);
	m_canvasHelperTL->setAnchor(0, 0);

	m_canvasHelperTR = new CBaseUITextbox(1, 0, 0.4, 0.4, "HelperTR");
	m_canvasHelperTR->setText("Top Right");
	m_canvasHelperTR->setTextJustification(1);
	m_canvasHelperTR->setAnchor(1, 0);

	m_canvasHelperBL = new CBaseUITextbox(0, 1, 0.4, 0.4, "HelperBL");
	m_canvasHelperBL->setText("Bottom Left");
	m_canvasHelperBL->setTextJustification(1);
	m_canvasHelperBL->setAnchor(0, 1);

	m_canvasHelperBR = new CBaseUITextbox(1, 1, 0.4, 0.4, "HelperBR");
	m_canvasHelperBR->setText("Bottom Right");
	m_canvasHelperBR->setTextJustification(1);
	m_canvasHelperBR->setAnchor(1, 1);

	// Buttons
	m_canvasMainResize = new CBaseUIButton(300, 600, 300, 25, "canvasMainResize", "Resize Main Canvas");
	m_canvasMainResize->setClickCallback(fastdelegate::MakeDelegate(this, &UIFrameworkTestMultiCanvas::canvasMainResize));

	m_canvasMainMove = new CBaseUIButton(300, 500, 300, 25, "canvasMainMove", "Move Main Canvas");
	m_canvasMainMove->setClickCallback(fastdelegate::MakeDelegate(this, &UIFrameworkTestMultiCanvas::canvasMainMove));

	m_canvasInnerResize = new CBaseUIButton(300, 400, 300, 25, "canvasInnerResize", "Resize Inner Canvas");
	m_canvasInnerResize->setClickCallback(fastdelegate::MakeDelegate(this, &UIFrameworkTestMultiCanvas::canvasInnerResize));

	// Set UI Hierarchy
	// Top Left Canvas Helpers
	m_canvasTL->addSlot(m_canvasHelperTL);
	m_canvasTL->addSlot(m_canvasHelperTR);
	m_canvasTL->addSlot(m_canvasHelperBL);
	m_canvasTL->addSlot(m_canvasHelperBR);

	// Top Right Canvas Helpers
	m_canvasTR->addSlot(m_canvasHelperTL);
	m_canvasTR->addSlot(m_canvasHelperTR);
	m_canvasTR->addSlot(m_canvasHelperBL);
	m_canvasTR->addSlot(m_canvasHelperBR);

	// Bottom Left Canvas Helpers
	m_canvasBL->addSlot(m_canvasHelperTL);
	m_canvasBL->addSlot(m_canvasHelperTR);
	m_canvasBL->addSlot(m_canvasHelperBL);
	m_canvasBL->addSlot(m_canvasHelperBR);

	// Bottom Right Canvas Helpers
	m_canvasBR->addSlot(m_canvasHelperTL);
	m_canvasBR->addSlot(m_canvasHelperTR);
	m_canvasBR->addSlot(m_canvasHelperBL);
	m_canvasBR->addSlot(m_canvasHelperBR);

	// Inner Canvas
	m_canvasMain->addSlot(m_canvasTL);
	m_canvasMain->addSlot(m_canvasTR);
	m_canvasMain->addSlot(m_canvasBL);
	m_canvasMain->addSlot(m_canvasBR);


	m_bCanvasMainResize=false;
	m_bCanvasMainMove=false;
	m_bCanvasInnerResize=false;
}

UIFrameworkTestMultiCanvas::~UIFrameworkTestMultiCanvas() {
	m_canvasMain->clear();

	SAFE_DELETE(m_canvasMainResize);
	SAFE_DELETE(m_canvasMainMove);
	SAFE_DELETE(m_canvasInnerResize);
}

void UIFrameworkTestMultiCanvas::draw(Graphics *g){
	m_canvasMain->draw(g);
	m_canvasMainResize->draw(g);
	m_canvasMainMove->draw(g);
	m_canvasInnerResize->draw(g);

	m_canvasMain->drawDebug(g);
}

void UIFrameworkTestMultiCanvas::update(){
	m_canvasMain->update();
	m_canvasMainResize->update();
	m_canvasMainMove->update();
	m_canvasInnerResize->update();
}

void UIFrameworkTestMultiCanvas::canvasMainResize(){
	if (m_bCanvasMainResize){
		m_canvasMain->setSize(engine->getScreenSize());
		m_bCanvasMainResize=false;
	}
	else{
		m_canvasMain->setSize(800, 600);
		m_bCanvasMainResize=true;
	}

}

void UIFrameworkTestMultiCanvas::canvasMainMove(){
	if (m_bCanvasMainMove){
		m_canvasMain->setPos(0, 0);
		m_bCanvasMainMove=false;
	}
	else{
		m_canvasMain->setPos(150, 150);
		m_bCanvasMainMove=true;
	}
}

void UIFrameworkTestMultiCanvas::canvasInnerResize(){
	if (m_bCanvasInnerResize){
		m_canvasTL->setSize(0.5, 0.5);
		m_canvasTR->setSize(0.5, 0.5);
		m_canvasBL->setSize(0.5, 0.5);
		m_canvasBR->setSize(0.5, 0.5);

		m_bCanvasInnerResize=false;
	}

	else{
		m_canvasTL->setSize(0.25, 0.25);
		m_canvasTR->setSize(0.75, 0.3);
		m_canvasBL->setSize(0.4, 0.75);
		m_canvasBR->setSize(0.6, 0.7);

		m_bCanvasInnerResize=true;
	}
}
