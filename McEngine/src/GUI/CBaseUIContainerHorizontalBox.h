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
 * Use m_bScaleByHeightOnly to scale using the height of the container (assumed 16:9 desired aspect ratio)
 * No per-element scaling options yet.
 */

class CBaseUIContainerHorizontalBox : public CBaseUIContainerBase
{
public:
	CBaseUIContainerHorizontalBox(float posX=0, float posY=0, float sizeX=0, float sizeY=0, UString name="");
	virtual ~CBaseUIContainerHorizontalBox();

	CONTAINER_BODY(CBaseUIContainerHorizontalBox)

protected:
	virtual void updateLayout();
	virtual void updateElement(CBaseUIElement *element);

	bool m_bWidthOverride;
	bool m_bScaleByHeightOnly;
};

#endif /* GUI_CBASEUICONTAINERHORIZONTALBOX_H_ */
