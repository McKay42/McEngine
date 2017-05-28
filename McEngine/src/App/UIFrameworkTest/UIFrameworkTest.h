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

class UIFrameworkTest: public App {
public:
	UIFrameworkTest();
	virtual ~UIFrameworkTest();

	virtual void draw(Graphics *g);
	virtual void update();

public:
	// UI
	CBaseUIButton *m_anchorTestButton;
};

#endif /* APP_UIFRAMEWORKTEST_UIFRAMEWORKTEST_H_ */
