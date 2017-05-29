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

	// Anchor Test
	void changeTestAnchor();

	// Canvas Test
	void resizeCanvas();
	void moveCanvas();

public:
	// UI
	// Anchor Test
	CBaseUIButton *m_anchorTestButton;
	bool m_bAnchorChanged;

	// Canvas Test
	CBaseUICanvas *m_canvasTest;
	CBaseUITextbox *m_canvasTestTL;
	CBaseUITextbox *m_canvasTestTR;
	CBaseUITextbox *m_canvasTestBL;
	CBaseUITextbox *m_canvasTestBR;
	CBaseUIButton *m_canvasResizeButton;
	CBaseUIButton *m_canvasMoveButton;
	bool m_bCanvasResized;
	bool m_bCanvasMoved;


};

#endif /* APP_UIFRAMEWORKTEST_UIFRAMEWORKTEST_H_ */
