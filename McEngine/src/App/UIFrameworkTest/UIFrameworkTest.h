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
class CBaseUICanvas;
class CBaseUITextbox;

class UIFrameworkTest: public App {
public:
	UIFrameworkTest();
	virtual ~UIFrameworkTest();

	virtual void draw(Graphics *g);
	virtual void update();

	void resizeCanvas();

public:
	// UI
	CBaseUIButton *m_anchorTestButton;

	CBaseUICanvas *m_canvasTest;
	CBaseUITextbox *m_canvasTestTL;
	CBaseUITextbox *m_canvasTestTR;
	CBaseUITextbox *m_canvasTestBL;
	CBaseUITextbox *m_canvasTestBR;
	CBaseUIButton *m_canvasTestButton;
	bool m_bResized;


};

#endif /* APP_UIFRAMEWORKTEST_UIFRAMEWORKTEST_H_ */
