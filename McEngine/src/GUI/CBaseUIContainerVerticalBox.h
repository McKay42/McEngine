/*
 * CBaseUIContainerVerticalBox.h
 *
 *  Created on: Jun 7, 2017
 *      Author: Psy
 */

#ifndef GUI_CBASEUICONTAINERVERTICALBOX_H_
#define GUI_CBASEUICONTAINERVERTICALBOX_H_

#include "CBaseUIContainerBase.h"

/*
 * Container that arranges elements vertically
 * Be default, the container will stretch the elements to fit the space
 * Use m_bWidthOverride to use the relative width of elements
 * Use m_bScaleByHeightOnly to scale width using the height of the container (assumed 16:9 desired aspect ratio)
 * No per-element scaling options yet.
 */

class CBaseUIContainerVerticalBox : public CBaseUIContainerBase
{
public:
	CBaseUIContainerVerticalBox(float posX=0, float posY=0, float sizeX=0, float sizeY=0, UString name="");
	virtual ~CBaseUIContainerVerticalBox();

	CONTAINER_BODY(CBaseUIContainerVerticalBox)

	virtual void drawDebug(Graphics *g, Color color=COLOR(255,255,0,0));

	CBaseUIContainerVerticalBox *setPadding(float padding) {m_padding = padding; return this;}
	CBaseUIContainerVerticalBox *setMargin(float marginX, float marginY) {m_vMargin.x = marginX; m_vMargin.y = marginY; return this;}
	CBaseUIContainerVerticalBox *setMarginX(float marginX) {m_vMargin.x = marginX; return this;}
	CBaseUIContainerVerticalBox *setMarginY(float marginY) {m_vMargin.y = marginY; return this;}
	CBaseUIContainerVerticalBox *setMargin(Vector2 margin) {m_vMargin = margin; return this;}
	CBaseUIContainerVerticalBox *setMargin(float margin) {m_vMargin.x = m_vMargin.y = margin; return this;}
	CBaseUIContainerVerticalBox *setHeightOverride(bool heightOverride) {m_bHeightOverride = heightOverride; return this;}

protected:
	// events
	virtual void updateLayout();
	virtual void updateElement(CBaseUIElement *element);
	virtual void onMoved();
	virtual void onResized();

	// container options
	bool m_bHeightOverride;
	float m_padding;
	Vector2 m_vMargin;
};

#endif /* GUI_CBASEUICONTAINERVERTICALBOX_H_ */
