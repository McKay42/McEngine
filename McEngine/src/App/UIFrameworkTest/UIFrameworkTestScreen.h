/*
 * UIFrameworkTestScreen.h
 *
 *  Created on: May 29, 2017
 *      Author: Psy
 */

#ifndef APP_UIFRAMEWORKTEST_UIFRAMEWORKTESTSCREEN_H_
#define APP_UIFRAMEWORKTEST_UIFRAMEWORKTESTSCREEN_H_

#include "cbase.h"

class UIFrameworkTest;

class UIFrameworkTestScreen
{
public:
	UIFrameworkTestScreen(UIFrameworkTest *app);
	virtual ~UIFrameworkTestScreen();

	virtual void draw(Graphics *g) = 0;
	virtual void update() = 0;

	virtual void onResolutionChanged(Vector2 newResolution) {;}

	UIFrameworkTest *m_app;
};

#endif /* APP_UIFRAMEWORKTEST_UIFRAMEWORKTESTSCREEN_H_ */
