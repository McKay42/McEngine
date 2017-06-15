/*
 * UIFrameworkTestVerticalBox.h
 *
 *  Created on: Jun 7, 2017
 *      Author: Psy
 */

#ifndef APP_UIFRAMEWORKTEST_UIFRAMEWORKTESTVerticalBOX_H_
#define APP_UIFRAMEWORKTEST_UIFRAMEWORKTESTVerticalBOX_H_

#include "UIFrameworkTestScreen.h"

class UIFrameworkTest;
class CBaseUIButton;
class CBaseUITextbox;
class CBaseUIContainerVerticalBox;
class CBaseUILabel;

class UIFrameworkTestVerticalBox: public UIFrameworkTestScreen
{
public:
	UIFrameworkTestVerticalBox(UIFrameworkTest *app);
	virtual ~UIFrameworkTestVerticalBox();

	virtual void draw(Graphics *g);
	virtual void update();

	void resizeBoxesWidth();
	void resizeBoxesHeight();
	void addElement();
	void removeElement();

protected:
	std::shared_ptr<CBaseUIContainerVerticalBox> m_boxNormal;
	std::shared_ptr<CBaseUIContainerVerticalBox> m_boxWithPadding;
	std::shared_ptr<CBaseUIContainerVerticalBox> m_boxHeightOverride;
	std::shared_ptr<CBaseUIContainerVerticalBox> m_boxSizeByHeightOnly;

	std::shared_ptr<CBaseUILabel> m_boxNormalLabel;
	std::shared_ptr<CBaseUILabel> m_boxWithPaddingLabel;
	std::shared_ptr<CBaseUILabel> m_boxHeightOverrideLabel;
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

#endif /* APP_UIFRAMEWORKTEST_UIFRAMEWORKTESTVerticalBOX_H_ */

