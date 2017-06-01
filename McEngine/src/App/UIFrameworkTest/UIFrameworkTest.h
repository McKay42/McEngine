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
	UIFrameworkTestScreen *m_currentScreen;
	UIFrameworkTestScreen *m_menuScreen;
	UIFrameworkTestScreen *m_singleCanvasScreen;
	UIFrameworkTestScreen *m_multiCanvasScreen;

	CBaseUIButton *m_menuButton;
};

#endif /* APP_UIFRAMEWORKTEST_UIFRAMEWORKTEST_H_ */
