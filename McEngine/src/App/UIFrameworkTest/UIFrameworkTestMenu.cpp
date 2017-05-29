/*
 * UIFrameworkTestMenu.cpp
 *
 *  Created on: May 29, 2017
 *      Author: Psy
 */

#include "UIFrameworkTestMenu.h"
#include "CBaseUICanvas.h"
#include "CBaseUIButton.h"
#include "UIFrameworkTest.h"

UIFrameworkTestMenu::UIFrameworkTestMenu(UIFrameworkTest *app) : UIFrameworkTestScreen(app) {

	m_UIFrameworkTestSingleCanvas = new CBaseUIButton(100, 100, 200, 25, "UIFrameworkTestSingleCanvasButton", "Single Canvas");
	m_UIFrameworkTestSingleCanvas->setClickCallback(fastdelegate::MakeDelegate(this, &UIFrameworkTestMenu::loadUIFrameworkTestSingleCanvas));

	m_UIFrameworkTestMultiCanvas = new CBaseUIButton(100, 200, 200, 25, "UIFrameworkTestMultiCanvasButton", "Multi Canvas");
	m_UIFrameworkTestMultiCanvas->setClickCallback(fastdelegate::MakeDelegate(this, &UIFrameworkTestMenu::loadUIFrameworkTestMultiCanvas));
}

UIFrameworkTestMenu::~UIFrameworkTestMenu() {
	SAFE_DELETE(m_UIFrameworkTestSingleCanvas);
}

void UIFrameworkTestMenu::draw(Graphics *g){
	m_UIFrameworkTestSingleCanvas->draw(g);
	m_UIFrameworkTestMultiCanvas->draw(g);
}

void UIFrameworkTestMenu::update(){
	m_UIFrameworkTestSingleCanvas->update();
	m_UIFrameworkTestMultiCanvas->update();
}

void UIFrameworkTestMenu::loadUIFrameworkTestSingleCanvas(){
	m_app->m_currentScreen = m_app->m_singleCanvasScreen;
}

void UIFrameworkTestMenu::loadUIFrameworkTestMultiCanvas(){
	m_app->m_currentScreen = m_app->m_multiCanvasScreen;
}
