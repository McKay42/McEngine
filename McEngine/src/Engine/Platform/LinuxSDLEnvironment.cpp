//================ Copyright (c) 2022, PG, All rights reserved. =================//
//
// Purpose:		linux sdl environment
//
// $NoKeywords: $sdllinuxenv
//===============================================================================//

#ifdef __linux__

#include "LinuxSDLEnvironment.h"

#ifdef MCENGINE_FEATURE_SDL

#include <dirent.h>
#include <libgen.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>

LinuxSDLEnvironment::LinuxSDLEnvironment() : SDLEnvironment(NULL)
{

}

Environment::OS LinuxSDLEnvironment::getOS()
{
	return Environment::OS::OS_LINUX;
}

void LinuxSDLEnvironment::sleep(unsigned int us)
{
	usleep(us);
}

void LinuxSDLEnvironment::openURLInDefaultBrowser(UString url)
{
	if (fork() == 0)
		exit(execl("/usr/bin/xdg-open", "xdg-open", url.toUtf8(), (char*)0));
}

UString LinuxSDLEnvironment::getUsername()
{
	passwd *pwd = getpwuid(getuid());
	if (pwd != NULL && pwd->pw_name != NULL)
		return UString(pwd->pw_name);
	else
		return UString("");
}

UString LinuxSDLEnvironment::getUserDataPath()
{
	passwd *pwd = getpwuid(getuid());
	if (pwd != NULL && pwd->pw_dir != NULL)
		return UString(pwd->pw_dir);
	else
		return UString("");
}

bool LinuxSDLEnvironment::directoryExists(UString directoryName)
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

bool LinuxSDLEnvironment::createDirectory(UString directoryName)
{
	return mkdir(directoryName.toUtf8(), DEFFILEMODE) != -1;
}

std::vector<UString> LinuxSDLEnvironment::getFilesInFolder(UString folder)
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

std::vector<UString> LinuxSDLEnvironment::getFoldersInFolder(UString folder)
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

std::vector<UString> LinuxSDLEnvironment::getLogicalDrives()
{
	std::vector<UString> drives;
	drives.push_back(UString("/"));
	return drives;
}

UString LinuxSDLEnvironment::getFolderFromFilePath(UString filepath)
{
	if (directoryExists(filepath)) // indirect check if this is already a valid directory (and not a file)
		return filepath;
	else
		return UString(dirname((char*)filepath.toUtf8()));
}



// helper functions

int LinuxSDLEnvironment::getFilesInFolderFilter(const struct dirent *entry)
{
	return 1;
}

int LinuxSDLEnvironment::getFoldersInFolderFilter(const struct dirent *entry)
{
	return 1;
}

#endif

#endif
