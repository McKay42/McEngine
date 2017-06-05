/*
 * CBaseUIContainerBox.h
 *
 *  Created on: Jun 1, 2017
 *      Author: Psy
 */

#ifndef GUI_CBASEUICONTAINERBOX_H_
#define GUI_CBASEUICONTAINERBOX_H_

#include "CBaseUIContainerBase.h"

class CBaseUIContainerBox : public CBaseUIContainerBase
{
public:
	CBaseUIContainerBox(float xPos=0, float yPos=0, UString name="");
	virtual ~CBaseUIContainerBox();

	CONTAINER_BODY(CBaseUIContainerBox)

protected:
	virtual void updateLayout();
	virtual void updateElement(CBaseUIElement *element);


};

#endif /* GUI_CBASEUICONTAINERBOX_H_ */
