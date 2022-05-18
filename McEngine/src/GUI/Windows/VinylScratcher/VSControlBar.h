//================ Copyright (c) 2014, PG, All rights reserved. =================//
//
// Purpose:		play/pause/forward/shuffle/repeat/eq/settings/volume/time bar
//
// $NoKeywords: $
//===============================================================================//

#ifndef VSCONTROLBAR_H
#define VSCONTROLBAR_H

#include "CBaseUIElement.h"

class McFont;

class CBaseUIContainer;
class CBaseUIButton;
class CBaseUICheckbox;
class CBaseUISlider;

class VSControlBar : public CBaseUIElement
{
public:
	VSControlBar(int x, int y, int xSize, int ySize, McFont *font);
	virtual ~VSControlBar();

	virtual void draw(Graphics *g);
	virtual void update();

	inline CBaseUISlider *getVolumeSlider() const {return m_volume;}
	inline CBaseUIButton *getPlayButton() const {return m_play;}
	inline CBaseUIButton *getPrevButton() const {return m_prev;}
	inline CBaseUIButton *getNextButton() const {return m_next;}
	inline CBaseUIButton *getInfoButton() const {return m_info;}

protected:
	virtual void onResized();
	virtual void onMoved();
	virtual void onFocusStolen();
	virtual void onEnabled();
	virtual void onDisabled();

private:
	void onRepeatCheckboxChanged(CBaseUICheckbox *box);
	void onShuffleCheckboxChanged(CBaseUICheckbox *box);
	void onVolumeChanged(UString oldValue, UString newValue);

	CBaseUIContainer *m_container;

	CBaseUISlider *m_volume;
	CBaseUIButton *m_play;
	CBaseUIButton *m_prev;
	CBaseUIButton *m_next;

	CBaseUIButton *m_info;

	CBaseUIButton *m_settings;
	CBaseUICheckbox *m_shuffle;
	CBaseUICheckbox *m_eq;
	CBaseUICheckbox *m_repeat;
};

#endif
