//================ Copyright (c) 2018, PG, All rights reserved. =================//
//
// Purpose:		macOS SDL environment
//
// $NoKeywords: $macsdlenv
//===============================================================================//

#ifdef __APPLE__

#include "MacOSSDLEnvironment.h"

#ifdef MCENGINE_FEATURE_SDL

#include "MacOSEnvironment.h"

MacOSSDLEnvironment::MacOSSDLEnvironment() : SDLEnvironment(NULL)
{
	m_environment = new MacOSEnvironment(NULL);
}

MacOSSDLEnvironment::~MacOSSDLEnvironment()
{
	SAFE_DELETE(m_environment);
}

Environment::OS MacOSSDLEnvironment::getOS()
{
	return Environment::OS::OS_MACOS;
}

void MacOSSDLEnvironment::sleep(unsigned int us)
{
	m_environment->sleep(us);
}

void MacOSSDLEnvironment::openURLInDefaultBrowser(UString url)
{
	m_environment->openURLInDefaultBrowser(url);
}

UString MacOSSDLEnvironment::getUsername()
{
	return m_environment->getUsername();
}

UString MacOSSDLEnvironment::getUserDataPath()
{
	return m_environment->getUserDataPath();
}

bool MacOSSDLEnvironment::directoryExists(UString directoryName)
{
	return m_environment->directoryExists(directoryName);
}

bool MacOSSDLEnvironment::createDirectory(UString directoryName)
{
	return m_environment->createDirectory(directoryName);
}

std::vector<UString> MacOSSDLEnvironment::getFilesInFolder(UString folder)
{
	return m_environment->getFilesInFolder(folder);
}

std::vector<UString> MacOSSDLEnvironment::getFoldersInFolder(UString folder)
{
	return m_environment->getFoldersInFolder(folder);
}

std::vector<UString> MacOSSDLEnvironment::getLogicalDrives()
{
	return m_environment->getLogicalDrives();
}

UString MacOSSDLEnvironment::getFolderFromFilePath(UString filepath)
{
	return m_environment->getFolderFromFilePath(filepath);
}

UString MacOSSDLEnvironment::openFileWindow(const char *filetypefilters, UString title, UString initialpath)
{
	return m_environment->openFileWindow(filetypefilters, title, initialpath);
}

UString MacOSSDLEnvironment::openFolderWindow(UString title, UString initialpath)
{
	return m_environment->openFolderWindow(title, initialpath);
}

#endif

#endif
