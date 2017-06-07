/*
 * CBaseUIContainerHorizontalBox.h
 *
 *  Created on: Jun 1, 2017
 *      Author: Psy
 */

#ifndef GUI_CBASEUICONTAINERHORIZONTALBOX_H_
#define GUI_CBASEUICONTAINERHORIZONTALBOX_H_

#include "CBaseUIContainerBase.h"

/*
 * Container that arranges elements horizontally
 * Be default, the container will stretch the elements to fit the space
 * Use m_bWidthOverride to use the relative width of elements
 * Use m_bScaleByHeightOnly to scale width using the height of the container (assumed 16:9 desired aspect ratio)
 * No per-element scaling options yet.
 */

class CBaseUIContainerHorizontalBox : public CBaseUIContainerBase
{
public:
	CBaseUIContainerHorizontalBox(float posX=0, float posY=0, float sizeX=0, float sizeY=0, UString name="");
	virtual ~CBaseUIContainerHorizontalBox();

	CONTAINER_BODY(CBaseUIContainerHorizontalBox)

	virtual void drawDebug(Graphics *g, Color color=COLOR(255,255,0,0));

	CBaseUIContainerHorizontalBox *setPadding(float padding) {m_padding = padding; return this;}
	CBaseUIContainerHorizontalBox *setMargin(float marginX, float marginY) {m_vMargin.x = marginX; m_vMargin.y = marginY; return this;}
	CBaseUIContainerHorizontalBox *setMarginX(float marginX) {m_vMargin.x = marginX; return this;}
	CBaseUIContainerHorizontalBox *setMarginY(float marginY) {m_vMargin.y = marginY; return this;}
	CBaseUIContainerHorizontalBox *setMargin(Vector2 margin) {m_vMargin = margin; return this;}
	CBaseUIContainerHorizontalBox *setMargin(float margin) {m_vMargin.x = m_vMargin.y = margin; return this;}
	CBaseUIContainerHorizontalBox *setWidthOverride(bool widthOverride) {m_bWidthOverride = widthOverride; return this;}

protected:
	// events
	virtual void updateLayout();
	virtual void updateElement(CBaseUIElement *element);
	virtual void onMoved();
	virtual void onResized();

	// container options
	bool m_bWidthOverride;
	float m_padding;
	Vector2 m_vMargin;
};

#endif /* GUI_CBASEUICONTAINERHORIZONTALBOX_H_ */
