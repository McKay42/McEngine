//================ Copyright (c) 2015, PG, All rights reserved. =================//
//
// Purpose:		linux
//
// $NoKeywords: $linuxenv
//===============================================================================//

#ifdef __linux__

#include "LinuxEnvironment.h"
#include "Engine.h"

#include "LinuxContextMenu.h"

#include <dirent.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/param.h>
#include <unistd.h>
#include <pwd.h>

#include <string.h>
#include <stdio.h>

ContextMenu *LinuxEnvironment::createContextMenu()
{
	return new LinuxContextMenu();
}

Environment::OS LinuxEnvironment::getOS()
{
	return Environment::OS::OS_LINUX;
}

void LinuxEnvironment::sleep(unsigned int us)
{
	usleep(us);
}

UString LinuxEnvironment::getExecutablePath()
{
	char buf[4096];
	memset(buf, '\0', 4096);
	if (readlink("/proc/self/exe", buf, 4095) != -1)
		return UString(buf);
	else
		return UString("");
}

void LinuxEnvironment::openURLInDefaultBrowser(UString url)
{
	if (fork() == 0)
		exit(execl("/usr/bin/xdg-open", "xdg-open", url.toUtf8(), (char*)0));
}

UString LinuxEnvironment::getUsername()
{
	passwd *pwd = getpwuid(getuid());
	if (pwd != NULL && pwd->pw_name != NULL)
		return UString(pwd->pw_name);
	else
		return UString("");
}

UString LinuxEnvironment::getUserDataPath()
{
	passwd *pwd = getpwuid(getuid());
	if (pwd != NULL && pwd->pw_dir != NULL)
		return UString(pwd->pw_dir);
	else
		return UString("");
}

bool LinuxEnvironment::fileExists(UString filename)
{
	return std::ifstream(filename.toUtf8()).good();
}

bool LinuxEnvironment::directoryExists(UString directoryName)
{
	DIR *dir = opendir(directoryName.toUtf8());
	if (dir)
	{
		closedir(dir);
		return true;
	}
	else if (ENOENT == errno) // not a directory
	{
	}
	else // something else broke
	{
	}
	return false;
}

bool LinuxEnvironment::createDirectory(UString directoryName)
{
	return mkdir(directoryName.toUtf8(), DEFFILEMODE) != -1;
}

bool LinuxEnvironment::renameFile(UString oldFileName, UString newFileName)
{
	return rename(oldFileName.toUtf8(), newFileName.toUtf8()) != -1;
}

bool LinuxEnvironment::deleteFile(UString filePath)
{
	return remove(filePath.toUtf8()) == 0;
}

std::vector<UString> LinuxEnvironment::getFilesInFolder(UString folder)
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

std::vector<UString> LinuxEnvironment::getFoldersInFolder(UString folder)
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

std::vector<UString> LinuxEnvironment::getLogicalDrives()
{
	std::vector<UString> drives;
	drives.push_back(UString("/"));
	return drives;
}

UString LinuxEnvironment::getFolderFromFilePath(UString filepath)
{
	if (directoryExists(filepath)) // indirect check if this is already a valid directory (and not a file)
		return filepath;
	else
		return UString(dirname((char*)filepath.toUtf8()));
}

UString LinuxEnvironment::getFileExtensionFromFilePath(UString filepath, bool includeDot)
{
	const int idx = filepath.findLast(".");
	if (idx != -1)
		return filepath.substr(idx+1);
	else
		return UString("");
}

void LinuxEnvironment::showMessageInfo(UString title, UString message)
{
	// TODO:
}

void LinuxEnvironment::showMessageWarning(UString title, UString message)
{
	// TODO:
}

void LinuxEnvironment::showMessageError(UString title, UString message)
{
	// TODO:
}

void LinuxEnvironment::showMessageErrorFatal(UString title, UString message)
{
	// TODO:
}

UString LinuxEnvironment::openFileWindow(const char *filetypefilters, UString title, UString initialpath)
{
	// TODO:
	return UString("");
}

UString LinuxEnvironment::openFolderWindow(UString title, UString initialpath)
{
	// TODO:
	return UString("");
}

// helper functions

int LinuxEnvironment::getFilesInFolderFilter(const struct dirent *entry)
{
	return 1;
}

int LinuxEnvironment::getFoldersInFolderFilter(const struct dirent *entry)
{
	return 1;
}

#endif
