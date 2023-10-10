//================ Copyright (c) 2011, PG, All rights reserved. =================//
//
// Purpose:		textbox + scrollview command suggestion list
//
// $NoKeywords: $
//===============================================================================//

#ifndef CONSOLEBOX_H
#define CONSOLEBOX_H

#include "CBaseUIElement.h"

#ifdef MCENGINE_FEATURE_MULTITHREADING

#include <mutex>
#include "WinMinGW.Mutex.h"

#endif

class CBaseUITextbox;
class CBaseUIButton;
class CBaseUIScrollView;
class CBaseUIBoxShadow;

class ConsoleBoxTextbox;

class ConsoleBox : public CBaseUIElement
{
public:
	ConsoleBox();
	virtual ~ConsoleBox();

	void draw(Graphics *g);
	void drawLogOverlay(Graphics *g);
	void update();

	void onKeyDown(KeyboardEvent &e);
	void onChar(KeyboardEvent &e);

	void onResolutionChange(Vector2 newResolution);

	void processCommand(UString command);
	void execConfigFile(UString filename);

	void log(UString text, Color textColor = 0xffffffff);

	// set
	void setRequireShiftToActivate(bool requireShiftToActivate) {m_bRequireShiftToActivate = requireShiftToActivate;}

	// get
	bool isBusy();
	bool isActive();

	// ILLEGAL:
	inline ConsoleBoxTextbox *getTextbox() const {return m_textbox;}

private:
	struct LOG_ENTRY
	{
		UString text;
		Color textColor;
	};

private:
	void onSuggestionClicked(CBaseUIButton *suggestion);

	void addSuggestion(const UString &text, const UString &helpText, const UString &command);
	void clearSuggestions();

	void show();
	void toggle(KeyboardEvent &e);

	float getAnimTargetY();

	float getDPIScale();

	int m_iSuggestionCount;
	int m_iSelectedSuggestion; // for up/down buttons

	ConsoleBoxTextbox *m_textbox;
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
	std::vector<LOG_ENTRY> m_log;
	McFont *m_logFont;

	std::vector<UString> m_commandHistory;
	int m_iSelectedHistory;

#ifdef MCENGINE_FEATURE_MULTITHREADING

	std::mutex m_logMutex;

#endif

};

#endif
