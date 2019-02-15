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

class HorizonSDLEnvironment : public SDLEnvironment
{
public:
	HorizonSDLEnvironment();
	virtual ~HorizonSDLEnvironment();

	virtual void update();

	// system
	virtual OS getOS();

	// user
	virtual UString getUsername();

	// file IO
	virtual std::vector<UString> getFilesInFolder(UString folder);
	virtual std::vector<UString> getFoldersInFolder(UString folder);
	virtual std::vector<UString> getLogicalDrives();
	virtual UString getFolderFromFilePath(UString filepath);

	// mouse
	Vector2 getMousePos();
	void setMousePos(int x, int y);

	// ILLEGAL:
	void showKeyboard();
	bool isDocked();
	int getMemAvailableMB();
	int getMemUsedMB();

private:
	bool m_bDocked;

	Vector2 m_vMousePos;

	uint32_t m_sensorHandles[4];
};

#endif

#endif

#endif
