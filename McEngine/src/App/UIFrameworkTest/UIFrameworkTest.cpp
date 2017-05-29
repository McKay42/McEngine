/*
 * UIFrameworkTest.cpp
 *
 *  Created on: May 28, 2017
 *      Author: Psy
 */

#include <UIFrameworkTest/UIFrameworkTest.h>

#include "CBaseUIButton.h"
#include "UIFrameworkTestScreen.h"
#include "UIFrameworkTestSingleCanvas.h"
#include "UIFrameworkTestMultiCanvas.h"
#include "UIFrameworkTestMenu.h"

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
	SAFE_DELETE(m_currentScreen);
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

void UIFrameworkTest::menuClick(){
	m_currentScreen = m_menuScreen;
}
