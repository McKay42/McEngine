/*
 * UIFrameworkTest.cpp
 *
 *  Created on: May 28, 2017
 *      Author: Psy
 */

#include "UIFrameworkTest.h"
#include "CBaseUIButton.h"
#include "UIFrameworkTestScreen.h"
#include "UIFrameworkTestSingleCanvas.h"
#include "UIFrameworkTestMultiCanvas.h"
#include "UIFrameworkTestMenu.h"

UIFrameworkTest::UIFrameworkTest()
{
	m_menuScreen = new UIFrameworkTestMenu(this);
	m_singleCanvasScreen = new UIFrameworkTestSingleCanvas(this);
	m_multiCanvasScreen = new UIFrameworkTestMultiCanvas(this);
	m_currentScreen = m_menuScreen;

	m_menuButton = new CBaseUIButton(0, 500, 200, 25, "Menu Button", "Menu");
	m_menuButton->setTextColor(COLOR(255, 255, 50, 50));
	m_menuButton->setFrameColor(COLOR(255, 255, 50, 50));
	m_menuButton->setClickCallback(fastdelegate::MakeDelegate(this, &UIFrameworkTest::menuClick));
}

UIFrameworkTest::~UIFrameworkTest()
{
	SAFE_DELETE(m_menuScreen);
	SAFE_DELETE(m_singleCanvasScreen);
	SAFE_DELETE(m_multiCanvasScreen);
}

void UIFrameworkTest::draw(Graphics *g)
{
	m_currentScreen->draw(g);

	if (m_currentScreen != m_menuScreen)
		m_menuButton->draw(g);
}

void UIFrameworkTest::update()
{
	m_currentScreen->update();

	if (m_currentScreen != m_menuScreen)
		m_menuButton->update();
}

void UIFrameworkTest::menuClick()
{
	m_currentScreen = m_menuScreen;
}
