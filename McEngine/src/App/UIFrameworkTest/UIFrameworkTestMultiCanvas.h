/*
 * UIFrameworktTestMultiCanvas.h
 *
 *  Created on: May 29, 2017
 *      Author: Psy
 */

#ifndef APP_UIFRAMEWORKTEST_UIFRAMEWORKTESTMULTICANVAS_H_
#define APP_UIFRAMEWORKTEST_UIFRAMEWORKTESTMULTICANVAS_H_

#include "UIFrameworkTestScreen.h"
#include "cbase.h"

class UIFrameworkTest;
class CBaseUICanvas;
class CBaseUIButton;
class CBaseUITextbox;

class UIFrameworkTestMultiCanvas: public UIFrameworkTestScreen
{
public:
	UIFrameworkTestMultiCanvas(UIFrameworkTest *app);
	virtual ~UIFrameworkTestMultiCanvas();

	virtual void draw(Graphics *g);
	virtual void update();
	virtual void onResolutionChanged(Vector2 newResolution);

	void canvasMainResize();
	void canvasMainMove();
	void canvasInnerResize();

public:
	// UI Elements
	CBaseUICanvas *m_canvasMain;

	CBaseUICanvas *m_canvasTL;
	CBaseUICanvas *m_canvasTR;
	CBaseUICanvas *m_canvasBL;
	CBaseUICanvas *m_canvasBR;

	CBaseUITextbox *m_canvasHelperTL_TL;
	CBaseUITextbox *m_canvasHelperTL_TR;
	CBaseUITextbox *m_canvasHelperTL_BL;
	CBaseUITextbox *m_canvasHelperTL_BR;

	CBaseUITextbox *m_canvasHelperTR_TL;
	CBaseUITextbox *m_canvasHelperTR_TR;
	CBaseUITextbox *m_canvasHelperTR_BL;
	CBaseUITextbox *m_canvasHelperTR_BR;

	CBaseUITextbox *m_canvasHelperBL_TL;
	CBaseUITextbox *m_canvasHelperBL_TR;
	CBaseUITextbox *m_canvasHelperBL_BL;
	CBaseUITextbox *m_canvasHelperBL_BR;

	CBaseUITextbox *m_canvasHelperBR_TL;
	CBaseUITextbox *m_canvasHelperBR_TR;
	CBaseUITextbox *m_canvasHelperBR_BL;
	CBaseUITextbox *m_canvasHelperBR_BR;

	CBaseUIButton *m_canvasMainResize;
	CBaseUIButton *m_canvasMainMove;
	CBaseUIButton *m_canvasInnerResize;

	bool m_bCanvasMainResize;
	bool m_bCanvasMainMove;
	bool m_bCanvasInnerResize;
};

#endif /* APP_UIFRAMEWORKTEST_UIFRAMEWORKTESTMULTICANVAS_H_ */
