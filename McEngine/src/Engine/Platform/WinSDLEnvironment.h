//================ Copyright (c) 2019, PG, All rights reserved. =================//
//
// Purpose:		todo
//
// $NoKeywords: $
//===============================================================================//

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)

#ifndef WINSDLENVIRONMENT_H
#define WINSDLENVIRONMENT_H

#include "SDLEnvironment.h"

#ifdef MCENGINE_FEATURE_SDL

#include <windows.h>

class WinSDLEnvironment : public SDLEnvironment
{
public:
	WinSDLEnvironment(SDL_Window *window);
	virtual ~WinSDLEnvironment();

	// system
	virtual OS getOS();

	// user
	virtual UString getUsername();

	// file IO
	virtual std::vector<UString> getFilesInFolder(UString folder);
	virtual std::vector<UString> getFoldersInFolder(UString folder);
	virtual std::vector<UString> getLogicalDrives();
	virtual UString getFolderFromFilePath(UString filepath);

private:
	void path_strip_filename(TCHAR *Path);
};

#endif

#endif

#endif
