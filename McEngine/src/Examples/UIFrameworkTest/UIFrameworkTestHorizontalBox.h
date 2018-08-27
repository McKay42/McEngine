/*
 * UIFrameworkTestHorizontalBox.h
 *
 *  Created on: Jun 7, 2017
 *      Author: Psy
 */

#ifndef APP_UIFRAMEWORKTEST_UIFRAMEWORKTESTHORIZONTALBOX_H_
#define APP_UIFRAMEWORKTEST_UIFRAMEWORKTESTHORIZONTALBOX_H_

#include "UIFrameworkTestScreen.h"

class UIFrameworkTest;
class CBaseUIButton;
class CBaseUITextbox;
class CBaseUIContainerHorizontalBox;
class CBaseUILabel;

class UIFrameworkTestHorizontalBox: public UIFrameworkTestScreen
{
public:
	UIFrameworkTestHorizontalBox(UIFrameworkTest *app);
	virtual ~UIFrameworkTestHorizontalBox();

	virtual void draw(Graphics *g);
	virtual void update();

	void resizeBoxesHeight();
	void resizeBoxesWidth();
	void addElement();
	void removeElement();

protected:
	std::shared_ptr<CBaseUIContainerHorizontalBox> m_boxNormal;
	std::shared_ptr<CBaseUIContainerHorizontalBox> m_boxWithPadding;
	std::shared_ptr<CBaseUIContainerHorizontalBox> m_boxWidthOverride;
	std::shared_ptr<CBaseUIContainerHorizontalBox> m_boxSizeByHeightOnly;

	std::shared_ptr<CBaseUILabel> m_boxNormalLabel;
	std::shared_ptr<CBaseUILabel> m_boxWithPaddingLabel;
	std::shared_ptr<CBaseUILabel> m_boxWidthOverrideLabel;
	std::shared_ptr<CBaseUILabel> m_boxSizeByHeightOnlyLabel;

	std::shared_ptr<CBaseUIButton> m_resizeBoxesHeight;
	std::shared_ptr<CBaseUIButton> m_resizeBoxesWidth;
	std::shared_ptr<CBaseUIButton> m_addElement;
	std::shared_ptr<CBaseUIButton> m_removeElement;

private:
	bool m_bHeightResized;
	bool m_bWidthResized;
	int m_elementCount;
};

#endif /* APP_UIFRAMEWORKTEST_UIFRAMEWORKTESTHORIZONTALBOX_H_ */
