/*
 * UIFrameworkTest.h
 *
 *  Created on: May 28, 2017
 *      Author: Psy
 */

#ifndef APP_UIFRAMEWORKTEST_UIFRAMEWORKTEST_H_
#define APP_UIFRAMEWORKTEST_UIFRAMEWORKTEST_H_

#include "App.h"

class CBaseUIButton;
class UIFrameworkTestScreen;

class UIFrameworkTest: public App
{
public:
	UIFrameworkTest();
	virtual ~UIFrameworkTest();

	virtual void draw(Graphics *g);
	virtual void update();
	virtual void onResolutionChanged(Vector2 newResolution);

	virtual void menuClick();

public:
	std::shared_ptr<UIFrameworkTestScreen> m_currentScreen;
	std::shared_ptr<UIFrameworkTestScreen> m_menuScreen;
	std::shared_ptr<UIFrameworkTestScreen> m_singleCanvasScreen;
	std::shared_ptr<UIFrameworkTestScreen> m_multiCanvasScreen;
	std::shared_ptr<UIFrameworkTestScreen> m_horizontalBoxScreen;
	std::shared_ptr<UIFrameworkTestScreen> m_verticalBoxScreen;

	std::shared_ptr<CBaseUIButton> m_menuButton;
};

#endif /* APP_UIFRAMEWORKTEST_UIFRAMEWORKTEST_H_ */
