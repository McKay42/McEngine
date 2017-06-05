/*
 * CBaseUICanvas.h
 *
 *  Created on: May 28, 2017
 *      Author: Psy
 */

#ifndef GUI_WINDOWS_CBASEUICANVAS_H_
#define GUI_WINDOWS_CBASEUICANVAS_H_

#include "cbase.h"
#include "CBaseUIContainerBase.h"

/*
 * UI Canvas Container
 * Scales any slotted containers or elements by the size of the canvas, useful for resolution scaling
 * The size/position of UI elements slotted should 0.0 to 1.0 as a percentage of the total screen area
 * Set scaleByHeightOnly per element to avoid stretching/squashing on aspect ratio changes. Uses a 16:9 (Widescreen) aspect ratio for assumed desired width
 */

class CBaseUICanvas : public CBaseUIContainerBase
{
public:
	CBaseUICanvas(float xPos=0, float yPos=0, float xSize=0, float ySize=0, UString name="");
	virtual ~CBaseUICanvas();

	CONTAINER_BODY(CBaseUICanvas)

	// main
	virtual void drawDebug(Graphics *g, Color color=COLOR(255,255,0,0));

protected:
	// events
	virtual void onMoved();
	virtual void onResized();
	virtual void updateLayout();
	virtual void updateElement(CBaseUIElement *element);
};

#endif /* GUI_WINDOWS_CBASEUICANVAS_H_ */
