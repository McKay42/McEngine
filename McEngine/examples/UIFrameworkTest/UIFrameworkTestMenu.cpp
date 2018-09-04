/*
 * UIFrameworkTestMenu.cpp
 *
 *  Created on: May 29, 2017
 *      Author: Psy
 */

#include "UIFrameworkTestMenu.h"
#include "UIFrameworkTest.h"

#include "CBaseUI.h"

UIFrameworkTestMenu::UIFrameworkTestMenu(UIFrameworkTest *app) : UIFrameworkTestScreen(app)
{
	m_UIFrameworkTestSingleCanvas = std::make_shared<UI::Button>(100, 100, 200, 25, "UIFrameworkTestSingleCanvasButton", "Single Canvas");
	m_UIFrameworkTestSingleCanvas->setClickCallback(fastdelegate::MakeDelegate(this, &UIFrameworkTestMenu::loadUIFrameworkTestSingleCanvas));

	m_UIFrameworkTestMultiCanvas = std::make_shared<UI::Button>(100, 200, 200, 25, "UIFrameworkTestMultiCanvasButton", "Multi Canvas");
	m_UIFrameworkTestMultiCanvas->setClickCallback(fastdelegate::MakeDelegate(this, &UIFrameworkTestMenu::loadUIFrameworkTestMultiCanvas));

	m_UIFrameworkTestHorizontalBox = std::make_shared<UI::Button>(100, 300, 200, 25, "UIFrameworkTestHorizontalBox", "Horizontal Box");
	m_UIFrameworkTestHorizontalBox->setClickCallback(fastdelegate::MakeDelegate(this, &UIFrameworkTestMenu::loadUIFrameworkTestHorizontalBox));

	m_UIFrameworkTestVerticalBox = std::make_shared<UI::Button>(100, 400, 200, 25, "UIFrameworkTestVerticalBox", "Vertical Box");
	m_UIFrameworkTestVerticalBox->setClickCallback(fastdelegate::MakeDelegate(this, &UIFrameworkTestMenu::loadUIFrameworkTestVerticalBox));
}

UIFrameworkTestMenu::~UIFrameworkTestMenu()
{

}

void UIFrameworkTestMenu::draw(Graphics *g)
{
	m_UIFrameworkTestSingleCanvas->draw(g);
	m_UIFrameworkTestMultiCanvas->draw(g);
	m_UIFrameworkTestHorizontalBox->draw(g);
	m_UIFrameworkTestVerticalBox->draw(g);
}

void UIFrameworkTestMenu::update()
{
	m_UIFrameworkTestSingleCanvas->update();
	m_UIFrameworkTestMultiCanvas->update();
	m_UIFrameworkTestHorizontalBox->update();
	m_UIFrameworkTestVerticalBox->update();
}

void UIFrameworkTestMenu::loadUIFrameworkTestSingleCanvas()
{
	m_app->m_currentScreen = m_app->m_singleCanvasScreen;
}

void UIFrameworkTestMenu::loadUIFrameworkTestMultiCanvas()
{
	m_app->m_currentScreen = m_app->m_multiCanvasScreen;
}

void UIFrameworkTestMenu::loadUIFrameworkTestHorizontalBox()
{
	m_app->m_currentScreen = m_app->m_horizontalBoxScreen;
}

void UIFrameworkTestMenu::loadUIFrameworkTestVerticalBox()
{
	m_app->m_currentScreen = m_app->m_verticalBoxScreen;
}
