/*
 * UIFrameworkTestMenu.h
 *
 *  Created on: May 29, 2017
 *      Author: Psy
 */

#ifndef APP_UIFRAMEWORKTEST_UIFRAMEWORKTESTMENU_H_
#define APP_UIFRAMEWORKTEST_UIFRAMEWORKTESTMENU_H_

#include "cbase.h"
#include "UIFrameworkTestScreen.h"


class CBaseUIButton;
class CBaseUICanvas;

class UIFrameworkTestMenu: public UIFrameworkTestScreen
{
public:
	UIFrameworkTestMenu(UIFrameworkTest *app);
	virtual ~UIFrameworkTestMenu();

	virtual void draw(Graphics *g);
	virtual void update();

	void loadUIFrameworkTestSingleCanvas();
	void loadUIFrameworkTestMultiCanvas();
	void loadUIFrameworkTestHorizontalBox();
	void loadUIFrameworkTestVerticalBox();

	std::shared_ptr<CBaseUIButton> m_UIFrameworkTestSingleCanvas;
	std::shared_ptr<CBaseUIButton> m_UIFrameworkTestMultiCanvas;
	std::shared_ptr<CBaseUIButton> m_UIFrameworkTestHorizontalBox;
	std::shared_ptr<CBaseUIButton> m_UIFrameworkTestVerticalBox;
};

#endif /* APP_UIFRAMEWORKTEST_UIFRAMEWORKTESTMENU_H_ */
