//================ Copyright (c) 2014, PG, All rights reserved. =================//
//
// Purpose:		windows
//
// $NoKeywords: $winenv
//===============================================================================//

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)

#include "WinEnvironment.h"
#include "Engine.h"
#include "Mouse.h"

#include <tchar.h>
#include <string>

#define WINDOW_RESIZABLE

bool g_bCursorVisible = true;

WinEnvironment::WinEnvironment(HWND hwnd, HINSTANCE hinstance)
{
	m_hwnd = hwnd;
	m_hInstance = hinstance;

	m_mouseCursor = m_mouseCursorArrow = LoadCursor(NULL, IDC_ARROW);
	m_bFullScreen = false;
	m_vWindowSize = getWindowSize();
	m_bCursorClipped = false;
}

void WinEnvironment::update()
{
	m_bIsCursorInsideWindow = Rect(0, 0, engine->getScreenWidth(), engine->getScreenHeight()).contains(getMousePos());
}

void WinEnvironment::shutdown()
{
	SendMessage(m_hwnd, WM_CLOSE, 0, 0);
}

bool WinEnvironment::fileExists(UString filename)
{
	WIN32_FIND_DATAW FindFileData;
	HANDLE handle = FindFirstFileW(filename.wc_str(), &FindFileData);
	if (handle == INVALID_HANDLE_VALUE)
		return std::ifstream(filename.toUtf8()).good();
	else
	{
		FindClose(handle);
		return true;
	}
}

UString WinEnvironment::getClipBoardText()
{
	UString result = "";
	HANDLE clip = NULL;
	if (OpenClipboard(NULL))
	{
		clip = GetClipboardData(CF_UNICODETEXT);
		wchar_t *pchData = (wchar_t*)GlobalLock(clip);

		if (pchData != NULL)
			result = UString(pchData);

		GlobalUnlock(clip);
		CloseClipboard();
	}
	return result;
}

void WinEnvironment::setClipBoardText(UString text)
{
	if (text.length() < 1) return;

	if (OpenClipboard(NULL))
	{
		EmptyClipboard();

		HGLOBAL hglbCopy;
		hglbCopy = GlobalAlloc(GMEM_MOVEABLE, ((text.length() + 1) * sizeof(WCHAR)));

		if (hglbCopy == NULL)
        {
			debugLog("ERROR: hglbCopy == NULL!\n");
            CloseClipboard();
            return;
        }

		LPWSTR  lptstrCopy;
		lptstrCopy = (LPWSTR)GlobalLock(hglbCopy);
		memcpy(lptstrCopy, text.wc_str(), (text.length() + 1) * sizeof(WCHAR) );
        lptstrCopy[(text.length() + 1) * sizeof(WCHAR)] = (WCHAR) 0;    // null

		GlobalUnlock(hglbCopy);
		SetClipboardData(CF_UNICODETEXT, hglbCopy);

		CloseClipboard();
	}
}

void WinEnvironment::openURLInDefaultBrowser(UString url)
{
	ShellExecuteW(m_hwnd, L"open", url.wc_str(), NULL, NULL, SW_SHOW);
}

UString WinEnvironment::openFileWindow(const char *filetypefilters, UString title, UString initialpath)
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

UString WinEnvironment::openFolderWindow(UString title, UString initialpath)
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

std::vector<UString> WinEnvironment::getFilesInFolder(UString folder)
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

std::vector<UString> WinEnvironment::getFoldersInFolder(UString folder)
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

std::vector<UString> WinEnvironment::getLogicalDrives()
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

UString WinEnvironment::getFolderFromFilePath(UString filepath)
{
	char *aString = (char*)filepath.toUtf8();
	path_strip_filename(aString);
	return aString;
}

UString WinEnvironment::getFileExtensionFromFilePath(UString filepath, bool includeDot)
{
	int idx = filepath.findLast(".");
	if (idx != -1)
		return filepath.substr(idx+1);
	else
		return UString("");
}

void WinEnvironment::showMessageInfo(UString title, UString message)
{
	bool wasFullscreen = m_bFullScreen;
	handleShowMessageFullscreen();
	MessageBoxW(m_hwnd, message.wc_str(), title.wc_str(), MB_ICONINFORMATION | MB_OK);
	if (wasFullscreen)
	{
		maximize();
		enableFullscreen();
	}
}

void WinEnvironment::showMessageWarning(UString title, UString message)
{
	bool wasFullscreen = m_bFullScreen;
	handleShowMessageFullscreen();
	MessageBoxW(m_hwnd, message.wc_str(), title.wc_str(), MB_ICONWARNING | MB_OK);
	if (wasFullscreen)
	{
		maximize();
		enableFullscreen();
	}
}

void WinEnvironment::showMessageError(UString title, UString message)
{
	bool wasFullscreen = m_bFullScreen;
	handleShowMessageFullscreen();
	MessageBoxW(m_hwnd, message.wc_str(), title.wc_str(), MB_ICONERROR | MB_OK);
	if (wasFullscreen)
	{
		maximize();
		enableFullscreen();
	}
}

void WinEnvironment::showMessageErrorFatal(UString title, UString message)
{
	bool wasFullscreen = m_bFullScreen;
	handleShowMessageFullscreen();
	MessageBox(m_hwnd, message.toUtf8(), title.toUtf8(), MB_ICONSTOP | MB_OK);
	if (wasFullscreen)
	{
		maximize();
		enableFullscreen();
	}
}

void WinEnvironment::focus()
{
	SetForegroundWindow(m_hwnd);
}

void WinEnvironment::center()
{
	MoveWindow(m_hwnd, GetSystemMetrics(SM_CXSCREEN)/2 - m_vWindowSize.x/2, GetSystemMetrics(SM_CYSCREEN)/2 - m_vWindowSize.y/2, m_vWindowSize.x, m_vWindowSize.y, FALSE);
}

void WinEnvironment::minimize()
{
	ShowWindow(m_hwnd, SW_MINIMIZE);
}

void WinEnvironment::maximize()
{
	ShowWindow(m_hwnd, SW_MAXIMIZE);
}

void WinEnvironment::enableFullscreen()
{
	if (m_bFullScreen) return;

	// backup screen size
	int width = GetSystemMetrics(SM_CXSCREEN);
	int height = GetSystemMetrics(SM_CYSCREEN);
	m_vLastWindowSize = m_vWindowSize;

	// get screen pos
	RECT rect;
	GetWindowRect(m_hwnd, &rect);
	m_vLastWindowPos.x = rect.left;
	m_vLastWindowPos.y = rect.top;

	// revert
	SetWindowLongPtr(m_hwnd, GWL_STYLE, getWindowStyleFullscreen());
	MoveWindow(m_hwnd, 0, 0, width, height, FALSE);

	m_bFullScreen = true;
}

void WinEnvironment::disableFullscreen()
{
	if (!m_bFullScreen) return;

	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = m_vLastWindowSize.x;
	rect.bottom = m_vLastWindowSize.y;

	// maximize
	SetWindowLongPtr(m_hwnd, GWL_STYLE, getWindowStyleWindowed());
	AdjustWindowRect(&rect, getWindowStyleWindowed(), FALSE);
	MoveWindow(m_hwnd, m_vLastWindowPos.x, m_vLastWindowPos.y, m_vLastWindowSize.x, m_vLastWindowSize.y, FALSE);

	m_bFullScreen = false;
}

void WinEnvironment::setWindowTitle(UString title)
{
	SetWindowText(m_hwnd, title.toUtf8());
}

void WinEnvironment::setWindowPos(int x, int y)
{
	RECT clientRect;
	GetClientRect(m_hwnd, &clientRect);
	m_vLastWindowSize = Vector2(clientRect.right,clientRect.bottom);

	MoveWindow(m_hwnd, x, y, m_vLastWindowSize.x, m_vLastWindowSize.y, FALSE);
}

void WinEnvironment::setWindowSize(int width, int height)
{
	// backup last position
	RECT rect;
	GetWindowRect(m_hwnd, &rect);
	m_vLastWindowPos.x = rect.left;
	m_vLastWindowPos.y = rect.top;

	// request window size based on client size
	rect = {0, 0, width, height};
	AdjustWindowRect(&rect, getWindowStyleWindowed(), FALSE);

	// remember and set the new size
	m_vWindowSize.x = rect.right - rect.left;
	m_vWindowSize.y = rect.bottom - rect.top;
	m_vLastWindowSize = m_vWindowSize;
	MoveWindow(m_hwnd, m_vLastWindowPos.x, m_vLastWindowPos.y, m_vWindowSize.x, m_vWindowSize.y, FALSE);
}

void WinEnvironment::setWindowGhostCorporeal(bool corporeal)
{
	LONG_PTR exStyle = 0;
	if (corporeal)
		exStyle = WS_EX_WINDOWEDGE;
	else
		exStyle = WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT;

	SetWindowLongPtr(m_hwnd, GWL_EXSTYLE, exStyle);
}

Vector2 WinEnvironment::getWindowPos()
{
	// this respects the window border, because the engine only works in client coordinates
	POINT p;
	p.x = 0;
	p.y = 0;
	ClientToScreen(m_hwnd, &p);
	return Vector2(p.x, p.y);
}

Vector2 WinEnvironment::getWindowSize()
{
	RECT clientRect;
	GetClientRect(m_hwnd, &clientRect);
	return Vector2(clientRect.right, clientRect.bottom);
}

Vector2 WinEnvironment::getNativeScreenSize()
{
	return Vector2(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
}

Rect WinEnvironment::getVirtualScreenRect()
{
	return Rect(GetSystemMetrics(SM_XVIRTUALSCREEN), GetSystemMetrics(SM_YVIRTUALSCREEN), GetSystemMetrics(SM_CXVIRTUALSCREEN), GetSystemMetrics(SM_CYVIRTUALSCREEN));
}

Rect WinEnvironment::getDesktopRect()
{
	HMONITOR monitor = MonitorFromWindow(m_hwnd, MONITOR_DEFAULTTONEAREST);

	MONITORINFO info;
	info.cbSize = sizeof(MONITORINFO);

	GetMonitorInfo(monitor, &info);

	return Rect(info.rcMonitor.left, info.rcMonitor.top, info.rcMonitor.right - info.rcMonitor.left, info.rcMonitor.bottom - info.rcMonitor.top);
}

bool WinEnvironment::isCursorInWindow()
{
	return m_bIsCursorInsideWindow;
}

bool WinEnvironment::isCursorClipped()
{
	return m_bCursorClipped;
}

bool WinEnvironment::isCursorVisible()
{
	return g_bCursorVisible;
}

Vector2 WinEnvironment::getMousePos()
{
	POINT mpos;
	if (GetCursorPos(&mpos) == 0)
	{
		debugLog("WinEnvironment Error: GetCursorPos() returned 0!\n");
		return Vector2();
	}
	ScreenToClient(m_hwnd, &mpos);
	return Vector2(mpos.x, mpos.y);
}

Rect WinEnvironment::getCursorClip()
{
	return m_cursorClip;
}

void WinEnvironment::setCursor(CURSORTYPE cur)
{
	switch (cur)
	{
	case CURSOR_NORMAL:
		m_mouseCursor = LoadCursor(NULL, IDC_ARROW);
		break;
	case CURSOR_WAIT:
		m_mouseCursor = LoadCursor(NULL, IDC_WAIT);
		break;
	case CURSOR_SIZE_H:
		m_mouseCursor = LoadCursor(NULL, IDC_SIZEWE);
		break;
	case CURSOR_SIZE_V:
		m_mouseCursor = LoadCursor(NULL, IDC_SIZENS);
		break;
	case CURSOR_SIZE_HV:
		m_mouseCursor = LoadCursor(NULL, IDC_SIZENESW);
		break;
	case CURSOR_SIZE_VH:
		m_mouseCursor = LoadCursor(NULL, IDC_SIZENWSE);
		break;
	case CURSOR_TEXT:
		m_mouseCursor = LoadCursor(NULL, IDC_IBEAM);
		break;
	default:
		m_mouseCursor = LoadCursor(NULL, IDC_ARROW);
		break;
	}

	SetCursor(m_mouseCursor);
}

void WinEnvironment::setCursorVisible(bool visible)
{
	g_bCursorVisible = visible; // notify main_Windows.cpp (for client area cursor invis)

	int check = ShowCursor(visible);
	if (!visible)
	{
		for (int i=0; i<=check; i++)
		{
			ShowCursor(visible);
		}
	}
	else if (check < 0)
	{
		for (int i=check; i<=0; i++)
		{
			ShowCursor(visible);
		}
	}
}

void WinEnvironment::setMousePos(int x, int y)
{
	POINT temp;
	temp.x = (LONG)x;
	temp.y = (LONG)y;
	ClientToScreen(m_hwnd,&temp);
	SetCursorPos((int)temp.x,(int)temp.y);
}

void WinEnvironment::setCursorClip(bool clip, Rect rect)
{
	m_bCursorClipped = clip;
	m_cursorClip = rect;

	if (clip)
	{
		RECT windowRect;

		if (rect.getWidth() == 0 && rect.getHeight() == 0)
		{
			RECT clientRect;
			GetClientRect(m_hwnd, &clientRect);

			POINT topLeft;
			topLeft.x = 0;
			topLeft.y = 0;
			ClientToScreen(m_hwnd, &topLeft);

			POINT bottomRight;
			bottomRight.x = clientRect.right;
			bottomRight.y = clientRect.bottom;
			ClientToScreen(m_hwnd, &bottomRight);

			windowRect.left = topLeft.x;
			windowRect.top = topLeft.y;
			windowRect.right = bottomRight.x;
			windowRect.bottom = bottomRight.y;

			m_cursorClip = Rect(0, 0, windowRect.right-windowRect.left, windowRect.bottom-windowRect.top);
		}

		// TODO: custom rect (only fullscreen works atm)

		ClipCursor(&windowRect);
	}
	else
		ClipCursor(NULL);
}



// helper functions

void WinEnvironment::path_strip_filename(TCHAR *Path)
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

void WinEnvironment::handleShowMessageFullscreen()
{
	if (m_bFullScreen)
	{
		disableFullscreen();

		// HACKHACK: force minimize + focus if in fullscreen
		// minimizing allows us to view MessageBox()es at all, and focus brings it into the foreground (because minimize also unfocuses)
		minimize();
		focus();
	}
}

long WinEnvironment::getWindowStyleWindowed()
{
	long style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;

#ifndef WINDOW_RESIZABLE
	style = style & (~WS_SIZEBOX);
#endif

	return style;
}

long WinEnvironment::getWindowStyleFullscreen()
{
	return WS_SYSMENU | WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE;
}

#endif
