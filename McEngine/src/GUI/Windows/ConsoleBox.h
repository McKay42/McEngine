//================ Copyright (c) 2011, PG, All rights reserved. =================//
//
// Purpose:		textbox + scrollview command suggestion list
//
// $NoKeywords: $
//===============================================================================//

#ifndef CONSOLEBOX_H
#define CONSOLEBOX_H

#include "cbase.h"
#include "CBaseUIElement.h"

class CBaseUITextbox;
class CBaseUIButton;
class CBaseUIScrollView;
class CBaseUIBoxShadow;

class ConsoleBox : public CBaseUIElement
{
public:
	ConsoleBox();
	virtual ~ConsoleBox();

	void draw(Graphics *g);
	void update();

	void onKeyDown(KeyboardEvent &e);
	void onChar(KeyboardEvent &e);

	void onResolutionChange(Vector2 newResolution);

	void processCommand(UString command);
	void execConfigFile(UString filename);

	void log(UString text);

	// set
	void setRequireShiftToActivate(bool requireShiftToActivate) {m_bRequireShiftToActivate = requireShiftToActivate;}

	// get
	bool isBusy();
	bool isActive();

private:
	void onSuggestionClicked(CBaseUIButton *suggestion);

	void addSuggestion(UString text);
	void clearSuggestions();

	int m_iSuggestionCount;
	int m_iSelectedSuggestion; // for up/down buttons

	CBaseUITextbox *m_textbox;
	CBaseUIScrollView *m_suggestion;
	std::vector<CBaseUIButton*> m_vSuggestionButtons;
	float m_fSuggestionY;

	bool m_bRequireShiftToActivate;
	bool m_bConsoleAnimateOnce;
	float m_fConsoleDelay;
	float m_fConsoleAnimation;
	bool m_bConsoleAnimateIn;
	bool m_bConsoleAnimateOut;

	bool m_bSuggestionAnimateIn;
	bool m_bSuggestionAnimateOut;
	float m_fSuggestionAnimation;

	float m_fLogTime;
	float m_fLogYPos;
	std::vector<UString> m_log;
	McFont *m_logFont;

	std::vector<UString> m_commandHistory;
	int m_iSelectedHistory;
};

#endif
