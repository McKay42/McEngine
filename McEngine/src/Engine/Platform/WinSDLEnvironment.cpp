//================ Copyright (c) 2019, PG, All rights reserved. =================//
//
// Purpose:		windows sdl environment
//
// $NoKeywords: $sdlwinenv
//===============================================================================//

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)

#include "WinSDLEnvironment.h"

#ifdef MCENGINE_FEATURE_SDL

#include "Engine.h"

#define SDL_VIDEO_DRIVER_WINDOWS	// HACKHACK
#undef SDL_VIDEO_DRIVER_COCOA		// HACKHACK
#include "SDL_syswm.h" // for SDL_GetWindowWMInfo()

#include <Lmcons.h>
#include <Shlobj.h>

#include <tchar.h>
#include <string>

WinSDLEnvironment::WinSDLEnvironment() : SDLEnvironment(NULL)
{

}

Environment::OS WinSDLEnvironment::getOS()
{
	return Environment::OS::OS_WINDOWS;
}

void WinSDLEnvironment::sleep(unsigned int us)
{
	Sleep(us/1000);
}

void WinSDLEnvironment::openURLInDefaultBrowser(UString url)
{
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	if (SDL_GetWindowWMInfo(m_window, &info) == SDL_TRUE)
	{
		ShellExecuteW(info.info.win.window, L"open", url.wc_str(), NULL, NULL, SW_SHOW);
	}
}

UString WinSDLEnvironment::getUsername()
{
	DWORD username_len = UNLEN+1;
	wchar_t username[username_len];

	if (GetUserNameW(username, &username_len))
		return UString(username);

	return UString("");
}

UString WinSDLEnvironment::getUserDataPath()
{
	wchar_t path[PATH_MAX];

	if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path)))
		return UString(path);

	return UString("");
}

bool WinSDLEnvironment::directoryExists(UString directoryName)
{
	DWORD dwAttrib = GetFileAttributesW(directoryName.wc_str());
	return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

bool WinSDLEnvironment::createDirectory(UString directoryName)
{
	return CreateDirectoryW(directoryName.wc_str(), NULL);
}

std::vector<UString> WinSDLEnvironment::getFilesInFolder(UString folder)
{
	folder.append("*.*");
	WIN32_FIND_DATAW data;
	std::wstring buffer;
	std::vector<UString> files;

	HANDLE handle = FindFirstFileW(folder.wc_str(), &data);

	while (true)
	{
		std::wstring filename(data.cFileName);

		if (filename != buffer)
		{
			buffer = filename;

			if (filename.length() > 0)
			{
				if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) //directory
				{
					///if (filename.length() > 0)
					///	folders.push_back(filename.c_str());
				}
				else // file
				{
					if (filename.length() > 0)
						files.push_back(filename.c_str());
				}
			}

			FindNextFileW(handle, &data);
		}
		else
			break;
	}

	FindClose(handle);
	return files;
}

std::vector<UString> WinSDLEnvironment::getFoldersInFolder(UString folder)
{
	folder.append("*.*");
	WIN32_FIND_DATAW data;
	std::wstring buffer;
	std::vector<UString> folders;

	HANDLE handle = FindFirstFileW(folder.wc_str(), &data);

	while (true)
	{
		std::wstring filename(data.cFileName);

		if (filename != buffer)
		{
			buffer = filename;

			if (filename.length() > 0)
			{
				if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) //directory
				{
					if (filename.length() > 0)
						folders.push_back(filename.c_str());
				}
				else // file
				{
					///if (filename.length() > 0)
					///	files.push_back(filename.c_str());
				}
			}

			FindNextFileW(handle, &data);
		}
		else
			break;
	}

	FindClose(handle);
	return folders;
}

std::vector<UString> WinSDLEnvironment::getLogicalDrives()
{
	std::vector<UString> drives;
	DWORD dwDrives = GetLogicalDrives();

	for (int i=0; i<26; i++)
	{
		// 26 letters in [A..Z] range
		if (dwDrives & 1)
		{
			UString driveName = UString::format("%c", 'A'+i);
			UString driveExecName = driveName;
			driveExecName.append(":/");

			UString driveNameForGetDriveFunction = driveName;
			driveNameForGetDriveFunction.append(":\\");

			DWORD attributes = GetFileAttributes(driveNameForGetDriveFunction.toUtf8());

			//debugLog("checking %s, type = %i, free clusters = %lu\n", driveNameForGetDriveFunction.toUtf8(), GetDriveType(driveNameForGetDriveFunction.toUtf8()), attributes);

			// check if the drive is valid, and if there is media in it (e.g. ignore empty dvd drives)
			if (GetDriveType(driveNameForGetDriveFunction.toUtf8()) > DRIVE_NO_ROOT_DIR && attributes != INVALID_FILE_ATTRIBUTES)
			{
				if (driveExecName.length() > 0)
					drives.push_back(driveExecName);
			}
		}

		dwDrives >>= 1; // next bit
	}

	return drives;
}

UString WinSDLEnvironment::getFolderFromFilePath(UString filepath)
{
	char *aString = (char*)filepath.toUtf8();
	path_strip_filename(aString);
	return aString;
}

UString WinSDLEnvironment::getFileNameFromFilePath(UString filePath)
{
	// TODO: use PathStripPath
	if (filePath.length() < 1) return filePath;

	const size_t lastSlashIndex = filePath.findLast("/");
	if (lastSlashIndex != std::string::npos)
		return filePath.substr(lastSlashIndex + 1);

	return filePath;
}

UString WinSDLEnvironment::openFileWindow(const char *filetypefilters, UString title, UString initialpath)
{
	disableFullscreen();

	OPENFILENAME fn;
	ZeroMemory(&fn, sizeof(fn));

	char fileNameBuffer[255];

	// fill it
	fn.lStructSize = sizeof(fn);
	fn.hwndOwner = NULL;
	fn.lpstrFile = fileNameBuffer;
	fn.lpstrFile[0] = '\0';
	fn.nMaxFile = sizeof(fileNameBuffer);
	fn.lpstrFilter = filetypefilters;
	fn.nFilterIndex = 1;
	fn.lpstrFileTitle = NULL;
	fn.nMaxFileTitle = 0;
	fn.lpstrTitle = title.length() > 1 ? title.toUtf8() : NULL;
	fn.lpstrInitialDir = initialpath.length() > 1 ? initialpath.toUtf8() : NULL;
	fn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_ENABLESIZING;

	// open the dialog
	GetOpenFileName(&fn);

	return fn.lpstrFile;
}

UString WinSDLEnvironment::openFolderWindow(UString title, UString initialpath)
{
	disableFullscreen();

	OPENFILENAME fn;
	ZeroMemory(&fn, sizeof(fn));

	char fileNameBuffer[255];

	fileNameBuffer[0] = 's';
	fileNameBuffer[1] = 'k';
	fileNameBuffer[2] = 'i';
	fileNameBuffer[3] = 'n';
	fileNameBuffer[4] = '.';
	fileNameBuffer[5] = 'i';
	fileNameBuffer[6] = 'n';
	fileNameBuffer[7] = 'i';
	fileNameBuffer[8] = '\0';

	// fill it
	fn.lStructSize = sizeof(fn);
	fn.hwndOwner = NULL;
	fn.lpstrFile = fileNameBuffer;
	///fn.lpstrFile[0] = '\0';
	fn.nMaxFile = sizeof(fileNameBuffer);
	fn.nFilterIndex = 1;
	fn.lpstrFileTitle = NULL;
	fn.lpstrTitle = title.length() > 1 ? title.toUtf8() : NULL;
	fn.lpstrInitialDir = initialpath.length() > 1 ? initialpath.toUtf8() : NULL;
	fn.Flags = OFN_PATHMUSTEXIST | OFN_ENABLESIZING;

	// open the dialog
	GetOpenFileName(&fn);

	return fn.lpstrFile;
}



// helper functions

void WinSDLEnvironment::path_strip_filename(TCHAR *Path)
{
	size_t Len = _tcslen(Path);

	if (Len==0)
		return;

	size_t Idx = Len-1;
	while (TRUE)
	{
		TCHAR Chr = Path[Idx];
		if (Chr==TEXT('\\')||Chr==TEXT('/'))
		{
			if (Idx==0||Path[Idx-1]==':')
				Idx++;

			break;
		}
		else if (Chr==TEXT(':'))
		{
			Idx++;
			break;
		}
		else
		{
			if (Idx==0)
				break;
			else
				Idx--;;
		};
	};

	Path[Idx] = TEXT('\0');
}

#endif

#endif
