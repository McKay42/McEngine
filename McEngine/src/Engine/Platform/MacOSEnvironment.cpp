//================ Copyright (c) 2017, PG, All rights reserved. =================//
//
// Purpose:		macOS
//
// $NoKeywords: $macenv
//===============================================================================//

#ifdef __APPLE__

#include "MacOSEnvironment.h"
#include "main_OSX_cpp.h"
#include "Engine.h"

#include <dirent.h>
#include <sys/stat.h>

extern bool g_bRunning;

MacOSEnvironment::MacOSEnvironment(MacOSWrapper *wrapper)
{
	m_wrapper = wrapper;

	m_bCursorRequest = false;
	m_bCursorReset = false;

	m_iCursorVisibleCounter = 0;
	m_iCursorInvisibleCounter = 0;

	m_bCursorVisible = true;
	m_bCursorVisibleInternalOverride = m_bCursorVisible;
	m_bIsCursorInsideWindow = false;
}

MacOSEnvironment::~MacOSEnvironment()
{
}

void MacOSEnvironment::update()
{
	// handle cursor changes
	if (!m_bCursorRequest)
	{
		if (m_bCursorReset)
		{
			m_bCursorReset = false;
			setCursor(CURSOR_NORMAL);
		}
	}
	m_bCursorRequest = false;

	// check if cursor is inside the engine window
	m_bIsCursorInsideWindow = McRect(0, 0, engine->getScreenWidth(), engine->getScreenHeight()).contains(getMousePos());

	// imitate windows logic, the cursor visibility setting only applies if the cursor is within the engine window
	if (!m_bIsCursorInsideWindow && !m_bCursorVisible && !m_bCursorVisibleInternalOverride)
		setCursorVisible(true, true);
	else if (m_bIsCursorInsideWindow && !m_bCursorVisible && m_bCursorVisibleInternalOverride)
		setCursorVisible(false, true);
}

void MacOSEnvironment::shutdown()
{
	g_bRunning = false;
}

UString MacOSEnvironment::getUsername()
{
	return UString(m_wrapper->getUsername());
}

void MacOSEnvironment::openURLInDefaultBrowser(UString url)
{
	m_wrapper->openURLInDefaultBrowser(url.toUtf8());
}

bool MacOSEnvironment::fileExists(UString filename)
{
	return (bool)(std::ifstream(filename.toUtf8()));
}

std::vector<UString> MacOSEnvironment::getFilesInFolder(UString folder)
{
	std::vector<UString> files;

	struct dirent **namelist;
	int n = scandir(folder.toUtf8(), &namelist, getFilesInFolderFilter, alphasort);
	if (n < 0)
	{
		///debugLog("LinuxEnvironment::getFilesInFolder() error, scandir() returned %i!\n", n);
		return files;
	}

	while (n--)
	{
		const char *name = namelist[n]->d_name;
		UString uName = UString(name);
		UString fullName = folder;
		fullName.append(uName);
		free(namelist[n]);

		struct stat stDirInfo;
		int lstatret = lstat(fullName.toUtf8(), &stDirInfo);
		if (lstatret < 0)
		{
			//perror (name);
			//debugLog("LinuxEnvironment::getFilesInFolder() error, lstat() returned %i!\n", lstatret);
			continue;
		}

		if (!S_ISDIR(stDirInfo.st_mode))
			files.push_back(uName);
	}
	free(namelist);

	return files;
}

std::vector<UString> MacOSEnvironment::getFoldersInFolder(UString folder)
{
	std::vector<UString> folders;

	struct dirent **namelist;
	int n = scandir(folder.toUtf8(), &namelist, getFoldersInFolderFilter, alphasort);
	if (n < 0)
	{
		///debugLog("LinuxEnvironment::getFilesInFolder() error, scandir() returned %i!\n", n);
		return folders;
	}

	while (n--)
	{
		const char *name = namelist[n]->d_name;
		UString uName = UString(name);
		UString fullName = folder;
		fullName.append(uName);
		free(namelist[n]);

		struct stat stDirInfo;
		int lstatret = lstat(fullName.toUtf8(), &stDirInfo);
		if (lstatret < 0)
		{
			///perror (name);
			///debugLog("LinuxEnvironment::getFilesInFolder() error, lstat() returned %i!\n", lstatret);
			continue;
		}

		if (S_ISDIR(stDirInfo.st_mode))
			folders.push_back(uName);
	}
	free(namelist);

	return folders;
}

UString MacOSEnvironment::getFileExtensionFromFilePath(UString filepath, bool includeDot)
{
	int idx = filepath.findLast(".");
	if (idx != -1)
		return filepath.substr(idx+1);
	else
		return UString("");
}

void MacOSEnvironment::showMessageInfo(UString title, UString message)
{
	m_wrapper->showMessageInfo(title.toUtf8(), message.toUtf8());
}

void MacOSEnvironment::showMessageWarning(UString title, UString message)
{
	m_wrapper->showMessageWarning(title.toUtf8(), message.toUtf8());
}

void MacOSEnvironment::showMessageError(UString title, UString message)
{
	m_wrapper->showMessageError(title.toUtf8(), message.toUtf8());
}

void MacOSEnvironment::showMessageErrorFatal(UString title, UString message)
{
	m_wrapper->showMessageErrorFatal(title.toUtf8(), message.toUtf8());
}

void MacOSEnvironment::minimize()
{
	m_wrapper->minimize();
}

void MacOSEnvironment::setWindowTitle(UString title)
{
	m_wrapper->setWindowTitle(title.toUtf8());
}

Vector2 MacOSEnvironment::getWindowSize()
{
	MacOSWrapper::VECTOR2 windowSize = m_wrapper->getWindowSize();
	return Vector2(windowSize.x, windowSize.y);
}

bool MacOSEnvironment::isCursorInWindow()
{
	return m_bIsCursorInsideWindow;
}
bool MacOSEnvironment::isCursorVisible()
{
	return m_bCursorVisible;
}

Vector2 MacOSEnvironment::getMousePos()
{
	MacOSWrapper::VECTOR2 mousePos = m_wrapper->getMousePos();
	return Vector2(mousePos.x, -mousePos.y + engine->getScreenHeight() - 1); // wtf apple: "The y coordinate in the returned point starts from a base of 1, not 0."
}

void MacOSEnvironment::setCursor(CURSORTYPE cur)
{
	// TODO: finish other cursor types
	switch (cur)
	{
	case CURSOR_TEXT:
		m_wrapper->setCursor(1);
		break;
	default:
		m_wrapper->setCursor(0);
		break;
	}

	m_bCursorReset = true;
	m_bCursorRequest = true;
}

void MacOSEnvironment::setCursorVisible(bool visible)
{
	setCursorVisible(visible, false);
}

void MacOSEnvironment::setCursorVisible(bool visible, bool internalOverride)
{
	m_wrapper->setCursorVisible(visible);

	if (visible)
		m_iCursorVisibleCounter++;
	else
		m_iCursorInvisibleCounter++;

	if (visible)
	{
		for (int i=0; i<m_iCursorInvisibleCounter-1; i++)
		{
			m_wrapper->setCursorVisible(visible);
		}
		m_iCursorInvisibleCounter = 0;
	}
	else
	{
		for (int i=0; i<m_iCursorVisibleCounter-1; i++)
		{
			m_wrapper->setCursorVisible(visible);
		}
		m_iCursorVisibleCounter = 0;
	}

	if (!internalOverride)
		m_bCursorVisible = visible;

	m_bCursorVisibleInternalOverride = visible;
}

void MacOSEnvironment::setMousePos(int x, int y)
{
	m_wrapper->setMousePos(x, y);
}

void MacOSEnvironment::setCursorClip(bool clip, McRect rect)
{
	// TODO:
}



// helper functions

int MacOSEnvironment::getFilesInFolderFilter(const struct dirent *entry)
{
	return 1;
}

int MacOSEnvironment::getFoldersInFolderFilter(const struct dirent *entry)
{
	return 1;
}

#endif
