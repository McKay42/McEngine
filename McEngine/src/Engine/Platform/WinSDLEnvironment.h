//================ Copyright (c) 2019, PG, All rights reserved. =================//
//
// Purpose:		windows sdl environment
//
// $NoKeywords: $sdlwinenv
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
	WinSDLEnvironment();
	virtual ~WinSDLEnvironment() {;}

	// system
	virtual OS getOS();
	virtual void sleep(unsigned int us);
	virtual void openURLInDefaultBrowser(UString url);

	// user
	virtual UString getUsername();
	virtual UString getUserDataPath();

	// file IO
	virtual bool directoryExists(UString directoryName);
	virtual bool createDirectory(UString directoryName);
	virtual std::vector<UString> getFilesInFolder(UString folder);
	virtual std::vector<UString> getFoldersInFolder(UString folder);
	virtual std::vector<UString> getLogicalDrives();
	virtual UString getFolderFromFilePath(UString filepath);
	virtual UString getFileNameFromFilePath(UString filePath);

	// dialogs & message boxes
	virtual UString openFileWindow(const char *filetypefilters, UString title, UString initialpath);
	virtual UString openFolderWindow(UString title, UString initialpath);

private:
	void path_strip_filename(TCHAR *Path);
};

#endif

#endif

#endif
