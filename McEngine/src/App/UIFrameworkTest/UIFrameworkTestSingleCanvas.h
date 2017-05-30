/*
 * UIFrameworkTestSingleCanvas.h
 *
 *  Created on: May 29, 2017
 *      Author: Psy
 */

#ifndef APP_UIFRAMEWORKTEST_UIFRAMEWORKTESTSINGLECANVAS_H_
#define APP_UIFRAMEWORKTEST_UIFRAMEWORKTESTSINGLECANVAS_H_

#include "cbase.h"
#include "UIFrameworkTestScreen.h"

class UIFrameworkTest;
class CBaseUIButton;
class CBaseUITextbox;
class CBaseUICanvas;

class UIFrameworkTestSingleCanvas: public UIFrameworkTestScreen
{
public:
	UIFrameworkTestSingleCanvas(UIFrameworkTest *app);
	virtual ~UIFrameworkTestSingleCanvas();

	virtual void draw(Graphics *g);
	virtual void update();

	void resizeCanvas();
	void moveCanvas();

protected:
	// Canvas Test
	CBaseUICanvas *m_canvasTest;
	CBaseUITextbox *m_canvasTestTL;
	CBaseUITextbox *m_canvasTestTR;
	CBaseUITextbox *m_canvasTestBL;
	CBaseUITextbox *m_canvasTestBR;
	CBaseUITextbox *m_canvasTestHeightOnly;
	CBaseUIButton *m_canvasResizeButton;
	CBaseUIButton *m_canvasMoveButton;
	bool m_bCanvasResized;
	bool m_bCanvasMoved;
};

#endif /* APP_UIFRAMEWORKTEST_UIFRAMEWORKTESTSINGLECANVAS_H_ */
