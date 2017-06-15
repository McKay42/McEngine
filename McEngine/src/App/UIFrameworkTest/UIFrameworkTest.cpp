/*
 * UIFrameworkTest.cpp
 *
 *  Created on: May 28, 2017
 *      Author: Psy
 */

#include "UIFrameworkTest.h"
#include "UIFrameworkTestScreen.h"
#include "UIFrameworkTestSingleCanvas.h"
#include "UIFrameworkTestMultiCanvas.h"
#include "UIFrameworkTestHorizontalBox.h"
#include "UIFrameworkTestVerticalBox.h"
#include "UIFrameworkTestMenu.h"

#include "CBaseUI.h"

UIFrameworkTest::UIFrameworkTest()
{
	m_menuScreen = std::shared_ptr<UIFrameworkTestMenu>(new UIFrameworkTestMenu(this));
	m_singleCanvasScreen = std::shared_ptr<UIFrameworkTestSingleCanvas>(new UIFrameworkTestSingleCanvas(this));
	m_multiCanvasScreen = std::shared_ptr<UIFrameworkTestMultiCanvas>(new UIFrameworkTestMultiCanvas(this));
	m_horizontalBoxScreen = std::shared_ptr<UIFrameworkTestHorizontalBox>(new UIFrameworkTestHorizontalBox(this));
	m_verticalBoxScreen = std::shared_ptr<UIFrameworkTestVerticalBox>(new UIFrameworkTestVerticalBox(this));
	m_currentScreen = m_menuScreen;

	m_menuButton = std::make_shared<UI::Button>(0, 500, 200, 25, "Menu Button", "Menu");
	m_menuButton->setTextColor(COLOR(255, 255, 50, 50));
	m_menuButton->setFrameColor(COLOR(255, 255, 50, 50));
	m_menuButton->setClickCallback(fastdelegate::MakeDelegate(this, &UIFrameworkTest::menuClick));
}

UIFrameworkTest::~UIFrameworkTest()
{

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

void UIFrameworkTest::onResolutionChanged(Vector2 newResolution)
{
	m_currentScreen->onResolutionChanged(newResolution);
}

void UIFrameworkTest::menuClick()
{
	m_currentScreen = m_menuScreen;
}
