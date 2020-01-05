//================ Copyright (c) 2019, PG, All rights reserved. =================//
//
// Purpose:		nintendo switch SDL environment
//
// $NoKeywords: $nxsdlenv
//===============================================================================//

#ifdef __SWITCH__

#ifndef HORIZONSDLENVIRONMENT_H
#define HORIZONSDLENVIRONMENT_H

#include "SDLEnvironment.h"

#ifdef MCENGINE_FEATURE_SDL

#define MCENGINE_HORIZON_SDL_NUM_SCANCODES_SWITCH 160

class ConVar;

class HorizonSDLEnvironment : public SDLEnvironment
{
public:
	HorizonSDLEnvironment();
	virtual ~HorizonSDLEnvironment();

	virtual void update();
	void update_before_winproc(); // HACKHACK: mouse/keyboard

	// system
	virtual OS getOS();
	virtual void sleep(unsigned int us);

	// user
	virtual UString getUsername();

	// file IO
	virtual std::vector<UString> getFilesInFolder(UString folder);
	virtual std::vector<UString> getFoldersInFolder(UString folder);
	virtual std::vector<UString> getLogicalDrives();
	virtual UString getFolderFromFilePath(UString filepath);

	// window
	int getDPI() {return 96;}

	// mouse
	Vector2 getMousePos();
	void setMousePos(int x, int y);

	// ILLEGAL:
	void showKeyboard();
	bool isDocked();
	int getMemAvailableMB();
	int getMemUsedMB();

private:
	static ConVar *m_mouse_sensitivity_ref;

	bool m_bDocked;

	Vector2 m_vMousePos;

	uint32_t m_sensorHandles[4];

	// HACKHACK: manual keyboard/mouse handling
	static uint8_t locks;
	static bool keystate[MCENGINE_HORIZON_SDL_NUM_SCANCODES_SWITCH];
	static uint64_t prev_buttons;
	float m_fLastMouseDeltaTime;
};

#endif

#endif

#endif
